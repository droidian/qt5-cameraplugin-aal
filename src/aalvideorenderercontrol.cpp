/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "aalvideorenderercontrol.h"
#include "aalcameraservice.h"
#include "aalviewfindersettingscontrol.h"

#include "media_signals.h"

#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>

#include <deviceinfo/deviceinfo.h>

#include <QAbstractVideoBuffer>
#include <QAbstractVideoSurface>
#include <QDebug>
#include <QTimer>
#include <QUrl>
#include <QVideoSurfaceFormat>
#include <QOpenGLContext>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>

#include <hybris/common/dlfcn.h>
#include <hybris/ui/ui_compatibility_layer.h>
#include <hardware/gralloc.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <dlfcn.h>
#include <memory>

#ifndef GL_TEXTURE_EXTERNAL_OES
#define GL_TEXTURE_EXTERNAL_OES 0x8D65
#endif

const int32_t GB_FORMAT = HAL_PIXEL_FORMAT_RGBA_8888;
const uint32_t GB_ALLOC_USAGE =
        GRALLOC_USAGE_HW_TEXTURE |
        GRALLOC_USAGE_SW_READ_OFTEN |
        GRALLOC_USAGE_SW_WRITE_NEVER;

class AalTextureBufferMapper {
public:
    AalTextureBufferMapper() :
        m_textureId(0),
        m_width(0),
        m_height(0),
        m_mapMode(QAbstractVideoBuffer::NotMapped)
    {}
    virtual ~AalTextureBufferMapper() {}

    void setTextureId(GLuint textureId) { m_textureId = textureId; }
    void setSize(const QSize& size)
    {
        m_width = size.width();
        m_height = size.height();
    }

    virtual QAbstractVideoBuffer::MapMode mapMode() const = 0;
    virtual uchar* map(QAbstractVideoBuffer::MapMode mode, int* numBytes, int* bytesPerLine) = 0;
    virtual void unmap() = 0;

protected:
    GLuint m_textureId;
    int m_width;
    int m_height;
    QAbstractVideoBuffer::MapMode m_mapMode;
};

class AalTextureBufferGraphicMapper : public AalTextureBufferMapper {
public:
    AalTextureBufferGraphicMapper() :
        AalTextureBufferMapper(),
        m_graphicBuffer(nullptr),
        m_vramAddr(nullptr),
        m_eglImage(EGL_NO_IMAGE_KHR)
    {
        eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
        eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");
        glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    }

    ~AalTextureBufferGraphicMapper()
    {
    }

    QAbstractVideoBuffer::MapMode mapMode() const override
    {
        return m_mapMode;
    }

    uchar* map(QAbstractVideoBuffer::MapMode mode, int* numBytes, int* bytesPerLine) override
    {
        if (mode != QAbstractVideoBuffer::ReadOnly) {
            qWarning() << "Tried to map in unsupported mode:" << mode;
            return nullptr;
        }

        if (m_width <= 0 || m_height <= 0) {
            qWarning() << "Tried to map buffer of invalid dimensions, cannot map memory.";
            return nullptr;
        }

        // Already mapped? Return the address then
        if (m_vramAddr) {
            const int stride = graphic_buffer_get_stride(m_graphicBuffer);
            m_mapMode = mode;
            *numBytes = m_width * stride * 4;
            *bytesPerLine = stride * 4;
            return (uchar*)m_vramAddr;
        }

        if (!(eglCreateImageKHR && eglDestroyImageKHR && glEGLImageTargetTexture2DOES)) {
            qWarning() << "EGLImageKHR functions not found, cannot map memory.";
            return nullptr;
        }

        if (!QOpenGLContext::currentContext()) {
            qWarning() << "OpenGL context is not current, cannot map memory.";
            return nullptr;
        }

        if (!m_shader.get() && !compileShaders()) {
            return nullptr;
        }

        QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();

        // Resize intermediate GraphicBuffer for accessing the texture through EGLImageKHR
        if (!m_graphicBuffer) {
            m_graphicBuffer = graphic_buffer_new_sized(m_width, m_height, GB_FORMAT, GB_ALLOC_USAGE);
        }

        // Create EGLImageKHR from the m_graphicBuffer for readback purposes
        EGLClientBuffer eglClientBuffer = (EGLClientBuffer)graphic_buffer_get_native_buffer(m_graphicBuffer);
        if (m_eglImage == EGL_NO_IMAGE_KHR) {
            EGLDisplay dpy = eglGetCurrentDisplay();
            EGLContext context = EGL_NO_CONTEXT;
            EGLint attrs[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
                               EGL_NONE };
            m_eglImage = eglCreateImageKHR(dpy, context, EGL_NATIVE_BUFFER_ANDROID, eglClientBuffer, attrs);
        }

        // Texture that is going to receive the viewfinder's data, bound with the EGLImage
        GLuint targetTexture;
        gl->glGenTextures(1, &targetTexture);
        gl->glBindTexture(GL_TEXTURE_EXTERNAL_OES, targetTexture);
        gl->glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, m_eglImage);

        GLuint fbo;
        gl->glGenFramebuffers(1, &fbo);
        gl->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_EXTERNAL_OES, targetTexture, 0);

        // Draw the target texture to copy the viewfinder texture into the EGLImage
        renderWithShader(gl);

        // Cleanup & finish drawing
        gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        gl->glDeleteFramebuffers(1, &fbo);
        gl->glBindTexture(GL_TEXTURE_2D, 0);
        gl->glDeleteTextures(1, &targetTexture);
        gl->glFinish();

        // Map pixel data from the GraphicBuffer
        graphic_buffer_lock(m_graphicBuffer, GRALLOC_USAGE_SW_READ_OFTEN, &m_vramAddr);
        if (!m_vramAddr) {
            qWarning() << "Failed to lock GraphicBuffer";
            graphic_buffer_free(m_graphicBuffer);
            m_graphicBuffer = nullptr;
            return nullptr;
        }

        const int stride = graphic_buffer_get_stride(m_graphicBuffer);
        m_mapMode = mode;
        *numBytes = m_height * stride * 4;
        *bytesPerLine = stride * 4;

        return (uchar*)m_vramAddr;
    }

    void unmap() override
    {
        if (!(eglCreateImageKHR && eglDestroyImageKHR && glEGLImageTargetTexture2DOES)) {
            qWarning() << "EGLImageKHR functions not found, cannot unmap memory.";
            return;
        }

        if (m_eglImage != EGL_NO_IMAGE_KHR) {
            eglDestroyImageKHR(eglGetCurrentDisplay(), m_eglImage);
            m_eglImage = EGL_NO_IMAGE_KHR;
        }

        if (m_graphicBuffer) {
            graphic_buffer_unlock(m_graphicBuffer);
            graphic_buffer_free(m_graphicBuffer);
            m_graphicBuffer = nullptr;
            m_vramAddr = nullptr;
        }
    }

private:
    void renderWithShader(QOpenGLFunctions* gl)
    {
        const auto width = m_width;
        const auto height = m_height;
        const int textureUnit = 1;

        static const GLfloat vertex_buffer_data[] = {
            -1,-1, 0,
            -1, 1, 0,
             1,-1, 0,
            -1, 1, 0,
             1,-1, 0,
             1, 1, 0
        };

        static const GLfloat texture_buffer_data[] = {
            0, 0,
            0, 1,
            1, 0,
            0, 1,
            1, 0,
            1, 1
        };

        QOpenGLVertexArrayObject vao;
        QOpenGLBuffer vertexBuffer;
        QOpenGLBuffer textureBuffer;

        m_shader->bind();

        gl->glActiveTexture(GL_TEXTURE0 + textureUnit);
        gl->glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_textureId);

        gl->glViewport(0, 0, width, height);

        vao.create();
        vao.bind();

        vertexBuffer.create();
        vertexBuffer.bind();
        vertexBuffer.allocate(vertex_buffer_data, sizeof(vertex_buffer_data));
        m_shader->setAttributeBuffer("vertexCoord", GL_FLOAT, 0, 3, 0);
        m_shader->enableAttributeArray("vertexCoord");
        vertexBuffer.release();

        textureBuffer.create();
        textureBuffer.bind();
        textureBuffer.allocate(texture_buffer_data, sizeof(texture_buffer_data));
        m_shader->setAttributeBuffer("textureCoord", GL_FLOAT, 0, 2, 0);
        m_shader->enableAttributeArray("textureCoord");
        textureBuffer.release();

        m_shader->setUniformValue("tex", textureUnit);

        gl->glDrawArrays(GL_TRIANGLES, 0, 6);

        m_shader->disableAttributeArray("textureCoord");
        m_shader->disableAttributeArray("vertexCoord");
        vertexBuffer.destroy();
        textureBuffer.destroy();

        vao.release();
        m_shader->release();

        gl->glActiveTexture(GL_TEXTURE0);
    }

    bool compileShaders()
    {
        static const GLchar* VERTEX_SHADER = {
            "#version 100\n"
            "attribute highp vec3 vertexCoord;\n"
            "attribute highp vec2 textureCoord;\n"
            "varying highp vec2 uv;\n"
            "\n"
            "void main() {\n"
            "    uv = textureCoord.xy;\n"
            "    gl_Position = vec4(vertexCoord,1.0);\n"
            "}\n"
        };

        static const GLchar* FRAGMENT_SHADER = {
            "#version 100\n"
            "#extension GL_OES_EGL_image_external : require\n"
            "uniform samplerExternalOES tex;\n"
            "varying highp vec2 uv;\n"
            "\n"
            "void main() {\n"
            "    gl_FragColor.bgra = texture2D(tex, uv).bgra;\n"
            "}\n"
        };

        auto program = std::make_shared<QOpenGLShaderProgram>();
        bool success = false;

        success = program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, VERTEX_SHADER);
        if (!success) {
            qWarning() << "Failed to compile vertex shader. Reason:" << program->log();
            return false;
        }

        success = program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, FRAGMENT_SHADER);
        if (!success) {
            qWarning() << "Failed to compile fragment shader. Reason:" << program->log();
            return false;
        }

        success = program->link();
        if (!success) {
            qWarning() << "Failed to link shader. Reason:" << program->log();
            return false;
        }

        m_shader = program;
        return true;
    }

    std::shared_ptr<QOpenGLShaderProgram> m_shader;
    struct graphic_buffer* m_graphicBuffer;
    void* m_vramAddr;
    EGLImageKHR m_eglImage;

    PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
    PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
};

class AalTextureBufferPixelReadMapper : public AalTextureBufferMapper {
public:
    AalTextureBufferPixelReadMapper() :
        AalTextureBufferMapper(),
        m_pixelBuffer(nullptr)
    {
    }

    QAbstractVideoBuffer::MapMode mapMode() const override
    {
        return m_mapMode;
    }

    uchar* map(QAbstractVideoBuffer::MapMode mode, int* numBytes, int* bytesPerLine) override
    {
        if (mode != QAbstractVideoBuffer::ReadOnly) {
            qWarning() << "Tried to map in unsupported mode:" << mode;
            return nullptr;
        }

        if (m_width <= 0 || m_height <= 0) {
            qWarning() << "Tried to map buffer of invalid dimensions, cannot map memory.";
            return nullptr;
        }

        if (!QOpenGLContext::currentContext()) {
            qWarning() << "OpenGL context is not current, cannot map memory.";
            return nullptr;
        }

        // The only supported mode is read-only, hence re-create a new byte array
        // on subsequent map operations on the same mapper object.
        if (m_pixelBuffer) {
            delete[] m_pixelBuffer;
        }
        m_pixelBuffer = new uint8_t[m_width * m_height * 4];

        QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
        GLuint fbo;
        gl->glGenFramebuffers(1, &fbo);
        gl->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        gl->glBindTexture(GL_TEXTURE_EXTERNAL_OES, m_textureId);
        gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_EXTERNAL_OES, m_textureId, 0);
        gl->glReadPixels(0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, m_pixelBuffer);
        gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        gl->glBindTexture(GL_TEXTURE_2D, 0);
        gl->glDeleteFramebuffers(1, &fbo);

        m_mapMode = mode;
        *numBytes = m_width * m_height * 4;
        *bytesPerLine = m_width * 4;
        return m_pixelBuffer;
    }

    void unmap() override
    {
        if (m_pixelBuffer) {
            delete[] m_pixelBuffer;
            m_pixelBuffer = nullptr;
        }
    }

private:
    uchar* m_pixelBuffer;
};

class AalGLTextureBuffer : public QAbstractVideoBuffer
{
public:
    AalGLTextureBuffer(GLuint textureId, AalTextureBufferMapper* mapper) :
        QAbstractVideoBuffer(QAbstractVideoBuffer::GLTextureHandle),
        m_textureId(textureId),
        m_mapper(mapper)
    {
    }

    ~AalGLTextureBuffer()
    {
    }

    MapMode mapMode() const
    {
        if (!m_mapper)
            return QAbstractVideoBuffer::NotMapped;
        return m_mapper->mapMode();
    }

    uchar *map(MapMode mode, int *numBytes, int *bytesPerLine)
    {
        if (!m_mapper)
            return nullptr;
        return m_mapper->map(mode, numBytes, bytesPerLine);
    }

    void unmap()
    {
        if (!m_mapper)
            return;
        m_mapper->unmap();
    }

    QVariant handle() const
    {
        return QVariant::fromValue<unsigned int>(m_textureId);
    }

    GLuint textureId() { return m_textureId; }

private:
    GLuint m_textureId;
    AalTextureBufferMapper* m_mapper;
};


AalVideoRendererControl::AalVideoRendererControl(AalCameraService *service, QObject *parent)
    : QVideoRendererControl(parent)
    , m_surface(0),
      m_service(service),
      m_viewFinderRunning(false),
      m_previewStarted(false),
      m_textureId(0)
{
    {
        m_mapper = new AalTextureBufferPixelReadMapper();
    }

    // Get notified when qtvideo-node creates a GL texture
    connect(SharedSignal::instance(), SIGNAL(textureCreated(unsigned int)), this, SLOT(onTextureCreated(unsigned int)));
    connect(SharedSignal::instance(), SIGNAL(snapshotTaken(QImage)), this, SLOT(onSnapshotTaken(QImage)));

    qDebug() << SharedSignal::instance();
}

AalVideoRendererControl::~AalVideoRendererControl()
{
    if (m_mapper) {
        delete m_mapper;
        m_mapper = nullptr;
    }
}

QAbstractVideoSurface *AalVideoRendererControl::surface() const
{
    return m_surface;
}

void AalVideoRendererControl::setSurface(QAbstractVideoSurface *surface)
{
    if (m_surface != surface) {
        m_surface = surface;
        Q_EMIT surfaceChanged(surface);
    }
}

void AalVideoRendererControl::init(CameraControl *control, CameraControlListener *listener)
{
    Q_UNUSED(control);
    listener->on_preview_texture_needs_update_cb = &AalVideoRendererControl::updateViewfinderFrameCB;
    // ensures a new texture will be created by qtvideo-node
    m_textureId = 0;
}

void AalVideoRendererControl::startPreview()
{
    if (m_previewStarted) {
        return;
    }
    if (!m_service->androidControl()) {
        qWarning() << "Can't start preview without a CameraControl";
        return;
    }
    m_previewStarted = true;

    if (m_textureId) {
        CameraControl *cc = m_service->androidControl();
        android_camera_set_preview_texture(cc, m_textureId);
        android_camera_start_preview(cc);
    }

    // if no texture ID is set to the frame passed to ShaderVideoNode,
    // a texture ID will be generated and returned via the 'textureCreated' signal
    // after calling updateViewfinderFrame()
    updateViewfinderFrame();

    m_service->updateCaptureReady();
}

void AalVideoRendererControl::stopPreview()
{
    if (!m_previewStarted) {
        return;
    }
    if (!m_service->androidControl()) {
        qWarning() << "Can't stop preview without a CameraControl";
        return;
    }
    if (!m_surface) {
        qWarning() << "Can't stop preview without a QAbstractVideoSurface";
        return;
    }

    if (m_surface->isActive()) {
        m_surface->stop();
    }

    CameraControl *cc = m_service->androidControl();
    android_camera_stop_preview(cc);
    // FIXME: missing android_camera_set_preview_size(QSize())
    android_camera_set_preview_texture(cc, 0);

    m_previewStarted = false;
    m_service->updateCaptureReady();
}

bool AalVideoRendererControl::isPreviewStarted() const
{
    return m_previewStarted;
}

void AalVideoRendererControl::updateViewfinderFrame()
{
    if (!m_service->viewfinderControl()) {
        qWarning() << "Can't draw video frame without a viewfinder settings control";
        return;
    }
    if (!m_service->androidControl()) {
        qWarning() << "Can't draw video frame without camera";
        return;
    }
    if (!m_surface) {
        qWarning() << "Can't draw video frame without surface";
        return;
    }

    QSize vfSize = m_service->viewfinderControl()->currentSize();
    m_mapper->setTextureId(m_textureId);
    m_mapper->setSize(vfSize);
    QVideoFrame frame(new AalGLTextureBuffer(m_textureId, m_mapper), vfSize, QVideoFrame::Format_RGB32);

    if (!frame.isValid()) {
        qWarning() << "Invalid frame";
        return;
    }

    CameraControl *cc = m_service->androidControl();
    frame.setMetaData("CamControl", QVariant::fromValue((void*)cc));

    if (!m_surface->isActive()) {
        QVideoSurfaceFormat format(frame.size(), frame.pixelFormat(), frame.handleType());

        if (!m_surface->start(format)) {
            qWarning() << "Failed to start viewfinder with format:" << format;
        }
    }

    if (m_surface->isActive()) {
        m_surface->present(frame);
    }
}

void AalVideoRendererControl::onTextureCreated(GLuint textureID)
{
    m_textureId = textureID;
    CameraControl *cc = m_service->androidControl();
    if (cc) {
        android_camera_set_preview_texture(cc, m_textureId);
        if (m_textureId && m_previewStarted) {
            android_camera_start_preview(cc);
        }
    }
    m_service->updateCaptureReady();
}

void AalVideoRendererControl::onSnapshotTaken(QImage snapshotImage)
{
    m_preview = snapshotImage;
    Q_EMIT previewReady();
}

void AalVideoRendererControl::updateViewfinderFrameCB(void* context)
{
    Q_UNUSED(context);
    AalVideoRendererControl *self = AalCameraService::instance()->videoOutputControl();
    if (self->m_previewStarted) {
        QMetaObject::invokeMethod(self, "updateViewfinderFrame", Qt::QueuedConnection);
    }
}

const QImage &AalVideoRendererControl::preview() const
{
    return m_preview;
}

void AalVideoRendererControl::createPreview()
{
    if (!m_textureId || !m_service->androidControl())
        return;

    QSize vfSize = m_service->viewfinderControl()->currentSize();
    SharedSignal::instance()->setSnapshotSize(vfSize);
    SharedSignal::instance()->takeSnapshot(m_service->androidControl());
}
