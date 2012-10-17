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

#include "camera_compatibility_layer.h"
#include "camera_compatibility_layer_capabilities.h"

#include <GLES2/gl2.h>

#include <QAbstractVideoBuffer>
#include <QAbstractVideoSurface>
#include <QDebug>
#include <QUrl>
#include <QVideoSurfaceFormat>

class AalGLTextureBuffer : public QAbstractVideoBuffer
{
public:
    AalGLTextureBuffer(int textureId) :
        QAbstractVideoBuffer(QAbstractVideoBuffer::QAbstractVideoBuffer::GLTextureHandle),
        m_textureId(textureId)
    {
    }

    MapMode mapMode() const { return NotMapped; }
    uchar *map(MapMode mode, int *numBytes, int *bytesPerLine)
    {
//        Q_UNUSED(mode);
//        Q_UNUSED(numBytes);
//        Q_UNUSED(bytesPerLine);

        qDebug() << Q_FUNC_INFO << "|" << mode << "|" << *numBytes << "|" << *bytesPerLine;

        return (uchar*)0;
    }

    void unmap()
    {
        qDebug() << Q_FUNC_INFO;
    }

    QVariant handle() const
    {
        return m_textureId;
    }

private:
//    GLuint m_textureId;
    int m_textureId;
};


AalVideoRendererControl::AalVideoRendererControl(AalCameraService *service, QObject *parent)
   : QVideoRendererControl(parent)
   , m_surface(0),
     m_service(service),
     m_viewFinderWidth(960),
     m_viewFinderHeight(720)
{
    GLuint previewTextureId;
//    glGenTextures(1, &previewTextureId);
//    m_textureBuffer = new AalGLTextureBuffer(previewTextureId);

    m_service->listener()->on_preview_texture_needs_update_cb = &AalVideoRendererControl::updateViewfinderFrameCB;
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

void AalVideoRendererControl::startPreview()
{
    CameraControl *cc = m_service->cameraControl()->control();

    android_camera_dump_parameters(cc);

    android_camera_set_preview_size(cc, m_viewFinderWidth, m_viewFinderHeight);

//    CameraPixelFormat pixel_format;
//    android_camera_get_preview_format(cc, &pixel_format);
//    printf("Current preview pixel format: %d \n", pixel_format);
    android_camera_get_preview_size(cc, &m_viewFinderWidth, &m_viewFinderHeight);
    qDebug() << "Preview size: "<< m_viewFinderWidth << "x" << m_viewFinderHeight;

    android_camera_set_preview_texture(cc, m_textureBuffer->handle().toInt());
    android_camera_start_preview(cc);
}

void AalVideoRendererControl::updateViewfinderFrame()
{
    qDebug() << Q_FUNC_INFO;
    if (!m_surface)
        return;

    android_camera_update_preview_texture(m_service->cameraControl()->control());

    QVideoFrame frame(m_textureBuffer, QSize(m_viewFinderWidth,m_viewFinderHeight), QVideoFrame::Format_BGRA32);
    if (!frame.isValid())
        return;

    if (!m_surface->isActive()) {
        QVideoSurfaceFormat format(frame.size(), frame.pixelFormat());

        if (!m_surface->start(format)) {
            qWarning() << "Failed to start viewfinder with format:" << format;
        }
    }

    if (m_surface->isActive()) {
        m_surface->present(frame);
    }
}

void AalVideoRendererControl::updateViewfinderFrameCB(void* context)
{
    Q_UNUSED(context);
    qDebug() << Q_FUNC_INFO;
//    AalCameraService::instance()->videoOutputControl()->updateViewfinderFrame();
}

