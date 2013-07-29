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

#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>
#include <qtubuntu_media_signals.h>

#include <QAbstractVideoBuffer>
#include <QAbstractVideoSurface>
#include <QDebug>
#include <QTimer>
#include <QUrl>
#include <QVideoSurfaceFormat>

const int MIN_FRAME_COUNT = 8; // minimal number of frames to be ready for capture

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
     m_viewFinderRunning(false),
     m_textureId(0),
     m_frameCount(0)
{
    // Get notified when qtvideo-node creates a GL texture
    connect(SharedSignal::instance(), SIGNAL(textureCreated(unsigned int)), this, SLOT(onTextureCreated(unsigned int)));
    connect(SharedSignal::instance(), SIGNAL(snapshotTaken(QImage)), this, SLOT(onSnapshotTaken(QImage)));
}

AalVideoRendererControl::~AalVideoRendererControl()
{
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
    m_textureId = 0;
    m_frameCount = 0;
}

void AalVideoRendererControl::startPreview()
{
    if (m_frameCount > 0)
        return;
    if (m_viewFinderRunning)
        return;

    if (m_textureId) {
        CameraControl *cc = m_service->androidControl();
        if (cc) {
            android_camera_set_preview_texture(cc, m_textureId);
            android_camera_start_preview(cc);
            m_viewFinderRunning = true;
        }
    } else {
        // requests a new texture
        updateViewfinderFrame();
    }
    m_service->updateCaptureReady();
}

void AalVideoRendererControl::stopPreview()
{
    if (!m_viewFinderRunning || !m_surface)
        return;

    m_viewFinderRunning = false;

    CameraControl *cc = m_service->androidControl();
    if (cc) {
        android_camera_stop_preview(cc);
    }

    if (m_surface->isActive())
        m_surface->stop();

    m_frameCount = 0;

    m_service->updateCaptureReady();
}

void AalVideoRendererControl::updateViewfinderFrame()
{
    if (!m_surface) {
        qWarning() << "Can't draw video frame without surface";
        return;
    }
    if (!m_service->androidControl()) {
        qWarning() << "Can't draw video frame without camera";
        return;
    }
    // if framecount is 0, then m_textureId can be 0, as this will request the
    // texture from the render thread. (see code in qtvideo-node).
    if (!m_textureId && m_frameCount > 0) {
        qWarning() << "Can't draw video frame without texture" << m_frameCount;
        return;
    }

    QSize vfSize = m_service->viewfinderControl()->currentSize();
    QVideoFrame frame(new AalGLTextureBuffer(m_textureId), vfSize, QVideoFrame::Format_RGB32);

    if (!frame.isValid())
        return;

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

    ++m_frameCount;
    if (m_frameCount == MIN_FRAME_COUNT) {
        m_service->updateCaptureReady();
    }
}

void AalVideoRendererControl::onTextureCreated(GLuint textureID)
{
    m_textureId = textureID;
    if (m_textureId) {
        // as we got a new textureID
        CameraControl *cc = m_service->androidControl();
        if (cc) {
            android_camera_set_preview_texture(cc, m_textureId);
            android_camera_start_preview(cc);
            m_viewFinderRunning = true;
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
    if (self->m_viewFinderRunning)
        QMetaObject::invokeMethod(self, "updateViewfinderFrame", Qt::QueuedConnection);
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

bool AalVideoRendererControl::isViewfinderRunning() const
{
    return m_viewFinderRunning && m_frameCount >= MIN_FRAME_COUNT;
}
