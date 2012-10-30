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

#include "aalcamerazoomcontrol.h"
#include "aalcameraservice.h"

#include <QDebug>

#include "camera_compatibility_layer.h"
#include "camera_compatibility_layer_capabilities.h"

AalCameraZoomControl::AalCameraZoomControl(AalCameraService *service, QObject *parent)
    : QCameraZoomControl(parent),
      m_service(service),
      m_currentDigialZoom(1),
      m_maximalDigitalZoom(1),
      m_pendingZoom(-1),
      m_zoomRunning(false),
      m_stopping(false)
{
}

qreal AalCameraZoomControl::currentDigitalZoom() const
{
    return (qreal)m_currentDigialZoom;
}

qreal AalCameraZoomControl::currentOpticalZoom() const
{
    return 1.0;
}

qreal AalCameraZoomControl::maximumDigitalZoom() const
{
    return (qreal)m_maximalDigitalZoom;
}

qreal AalCameraZoomControl::maximumOpticalZoom() const
{
    return 1.0;
}

qreal AalCameraZoomControl::requestedDigitalZoom() const
{
    return (qreal)m_pendingZoom;
}

qreal AalCameraZoomControl::requestedOpticalZoom() const
{
    return 1.0;
}

void AalCameraZoomControl::zoomTo(qreal optical, qreal digital)
{
    QMutexLocker locker(&m_zoomMutex);
    Q_UNUSED(optical);

    if (digital < 1.0 || digital > m_maximalDigitalZoom) {
        qWarning() << "Invalid zoom value:" << digital;
        return;
    }

    if ( m_pendingZoom == static_cast<int>(digital))
        return;

    m_pendingZoom = static_cast<int>(digital);

    if (m_zoomRunning) {
        if (!m_stopping)
            android_camera_stop_zoom(m_service->androidControl());
        m_stopping = true;
        return;
    }

    if (m_pendingZoom == m_currentDigialZoom)
        return;

    android_camera_start_zoom(m_service->androidControl(), m_pendingZoom);
    m_zoomRunning = true;
}

void AalCameraZoomControl::setZoomFromHW(int zoomLevel)
{
    QMutexLocker locker(&m_zoomMutex);
    if (m_stopping) {
        m_stopping = false;
        if (zoomLevel == m_pendingZoom) {
            m_zoomRunning = false;
        } else {
            android_camera_start_zoom(m_service->androidControl(), m_pendingZoom);
            m_zoomRunning = true;
        }
        return;
    }

    if (zoomLevel == m_pendingZoom)
        m_zoomRunning = false;

    if (m_currentDigialZoom != zoomLevel) {
        m_currentDigialZoom = zoomLevel;
        Q_EMIT currentDigitalZoomChanged(m_currentDigialZoom);
    }
}

void AalCameraZoomControl::init(CameraControl *control, CameraControlListener *listener)
{
    listener->on_msg_zoom_cb = &AalCameraZoomControl::zoomCB;

    android_camera_get_max_zoom(control, &m_maximalDigitalZoom);
    Q_EMIT maximumDigitalZoomChanged(m_maximalDigitalZoom);
}

void AalCameraZoomControl::zoomCB(void* context, int newZoomLevel)
{
    Q_UNUSED(context);
    QMetaObject::invokeMethod(AalCameraService::instance()->zoomControl(),
                              "setZoomFromHW", Qt::QueuedConnection,
                              Q_ARG(int, newZoomLevel));
}
