/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Guenter Schwann <guenter.schwann@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "aalvideorenderercontrol.h"
#include "aalcameracontrol.h"
#include "aalcameraservice.h"
#include "snapshotgenerator.h"

#include "camera_compatibility_layer.h"
#include "camera_compatibility_layer_capabilities.h"

#include <qgl.h>

#include <QAbstractVideoBuffer>
#include <QAbstractVideoSurface>
#include <QDebug>
#include <QTimer>
#include <QUrl>
#include <QVideoSurfaceFormat>

class AalGLTextureBuffer : public QAbstractVideoBuffer
{
public:
    AalGLTextureBuffer(GLuint textureId) :
        QAbstractVideoBuffer(QAbstractVideoBuffer::GLTextureHandle),
        m_textureId(textureId)
    {
    }

    MapMode mapMode() const { return NotMapped; }
    uchar *map(MapMode mode, int *numBytes, int *bytesPerLine)
    {
        qDebug() << Q_FUNC_INFO;
        Q_UNUSED(mode);
        Q_UNUSED(numBytes);
        Q_UNUSED(bytesPerLine);
        return (uchar*)0;
    }

    void unmap()
    {
        qDebug() << Q_FUNC_INFO;
    }

    QVariant handle() const
    {
        return QVariant::fromValue<unsigned int>(m_textureId);
    }

    GLuint textureId() { return m_textureId; }

private:
    GLuint m_textureId;
};


AalVideoRendererControl::AalVideoRendererControl(AalCameraService *service, QObject *parent)
   : QVideoRendererControl(parent)
   , m_surface(0),
     m_service(service),
     m_textureBuffer(0),
     m_viewFinderWidth(1280),
     m_viewFinderHeight(720),
     m_viewFinderRunning(false)
{
    m_snapshotGenerator = new SnapshotGenerator;
    m_service->listener()->on_preview_texture_needs_update_cb = &AalVideoRendererControl::updateViewfinderFrameCB;
    QTimer::singleShot(1, this, SLOT(getTextureId())); // delay until mainloop is running (GL context exists)
}

AalVideoRendererControl::~AalVideoRendererControl()
{
    delete m_snapshotGenerator;

    if (m_textureBuffer) {
        GLuint textureId = m_textureBuffer->handle().toUInt();
        glDeleteTextures(1, &textureId);
        delete m_textureBuffer;
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

void AalVideoRendererControl::startPreview()
{
    m_viewFinderRunning = true;

    if (!m_textureBuffer)
        return;

    CameraControl *cc = m_service->androidControl();

    m_snapshotGenerator->setSize(m_viewFinderWidth, m_viewFinderHeight);
    android_camera_set_preview_size(cc, m_viewFinderWidth, m_viewFinderHeight);
    android_camera_set_preview_fps(cc, 30);

    android_camera_set_preview_texture(cc, m_textureBuffer->handle().toUInt());
    android_camera_start_preview(cc);
}

void AalVideoRendererControl::updateViewfinderFrame()
{
    if (!m_surface || !m_textureBuffer)
        return;

    static QVideoFrame frame(m_textureBuffer, QSize(m_viewFinderWidth,m_viewFinderHeight), QVideoFrame::Format_RGB32);

       if (!frame.isValid())
        return;

    CameraControl *cc = m_service->androidControl();
    frame.setMetaData("CamControl", (int)cc);

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

void AalVideoRendererControl::getTextureId()
{
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    if (textureId == 0) {
        qWarning() << "unanble to get texture ID";
        return;
    }

    m_textureBuffer = new AalGLTextureBuffer(textureId);
    startPreview();
}

void AalVideoRendererControl::updateViewfinderFrameCB(void* context)
{
    Q_UNUSED(context);
    QMetaObject::invokeMethod(AalCameraService::instance()->videoOutputControl(),
                              "updateViewfinderFrame", Qt::QueuedConnection);
}

const QImage &AalVideoRendererControl::preview() const
{
    return m_preview;
}

void AalVideoRendererControl::createPreview()
{
    if (!m_textureBuffer)
        return;

    GLuint texId = m_textureBuffer->textureId();
    GLfloat textureMatrix[16];
    android_camera_get_preview_texture_transformation(m_service->androidControl(), textureMatrix);
    m_preview = m_snapshotGenerator->snapshot(texId, textureMatrix);
}
