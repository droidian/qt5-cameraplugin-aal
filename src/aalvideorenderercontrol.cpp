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
#include "aalimagecapturecontrol.h"
#include "aalvideodeviceselectorcontrol.h"
#include "snapshotgenerator.h"

#include "camera_compatibility_layer.h"
#include "camera_compatibility_layer_capabilities.h"

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
     m_viewFinderWidth(320),
     m_viewFinderHeight(240),
     m_viewFinderRunning(false),
     m_textureId(0)
{
    m_snapshotGenerator = new SnapshotGenerator;
}

AalVideoRendererControl::~AalVideoRendererControl()
{
    delete m_snapshotGenerator;

    if (m_textureId) {
        glDeleteTextures(1, &m_textureId);
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
    listener->on_preview_texture_needs_update_cb = &AalVideoRendererControl::updateViewfinderFrameCB;

    if (m_service->deviceSelector()->selectedDevice() == 0) {
        m_viewFinderWidth = 1280;
        m_viewFinderHeight = 720;
    } else {
        m_viewFinderWidth = 960;
        m_viewFinderHeight = 720;
    }

    m_snapshotGenerator->setSize(m_viewFinderWidth, m_viewFinderHeight);
    android_camera_set_preview_size(control, m_viewFinderWidth, m_viewFinderHeight);
    android_camera_set_preview_fps(control, 30);
}

void AalVideoRendererControl::startPreview()
{
    if (m_viewFinderRunning)
        return;

    // to make sure it's started in the main thread only, and the GL context exists
    QTimer::singleShot(1, this, SLOT(doStartPreview()));
}

void AalVideoRendererControl::stopPreview()
{
    if (!m_viewFinderRunning || !m_surface)
        return;

    CameraControl *cc = m_service->androidControl();
    if (cc) {
        android_camera_stop_preview(cc);
        android_camera_set_preview_texture(cc, 0);
    }

    if (m_surface->isActive())
        m_surface->stop();

    if (m_textureId)
        glDeleteTextures(1, &m_textureId);

    m_viewFinderRunning = false;

    m_service->imageCaptureControl()->updateReady();
}

void AalVideoRendererControl::updateViewfinderFrame()
{
    if (!m_surface || !m_textureId || !m_service->androidControl())
        return;

    QVideoFrame frame(new AalGLTextureBuffer(m_textureId),
                         QSize(m_viewFinderWidth,m_viewFinderHeight),
                         QVideoFrame::Format_RGB32);

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

void AalVideoRendererControl::doStartPreview()
{
    glGenTextures(1, &m_textureId);
    if (m_textureId == 0) {
        qWarning() << "unanble to get texture ID";
        return;
    }

    CameraControl *cc = m_service->androidControl();
    if (cc) {
        android_camera_set_preview_texture(cc, m_textureId);
        android_camera_start_preview(cc);
        m_viewFinderRunning = true;
    }
    m_service->imageCaptureControl()->updateReady();
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
    if (!m_textureId || !m_service->androidControl())
        return;

    GLfloat textureMatrix[16];
    android_camera_get_preview_texture_transformation(m_service->androidControl(), textureMatrix);
    m_preview = m_snapshotGenerator->snapshot(m_textureId, textureMatrix);
}

bool AalVideoRendererControl::isViewfinderRunning() const
{
    return m_viewFinderRunning;
}
