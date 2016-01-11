/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include "aalcamerazoomcontrol.h"
#include "aalcameracontrol.h"
#include "aalcameraservice.h"

#include <QDebug>

#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>

AalCameraZoomControl::AalCameraZoomControl(AalCameraService *service, QObject *parent)
    : QCameraZoomControl(parent),
      m_service(service),
      m_currentDigitalZoom(0),
      m_maximumDigitalZoom(1),
      m_pendingZoom(-1)
{
}

qreal AalCameraZoomControl::currentDigitalZoom() const
{
    return (qreal)m_currentDigitalZoom;
}

qreal AalCameraZoomControl::currentOpticalZoom() const
{
    return 0.0;
}

qreal AalCameraZoomControl::maximumDigitalZoom() const
{
    return (qreal)m_maximumDigitalZoom;
}

qreal AalCameraZoomControl::maximumOpticalZoom() const
{
    return 0.0;
}

qreal AalCameraZoomControl::requestedDigitalZoom() const
{
    return (qreal)m_pendingZoom;
}

qreal AalCameraZoomControl::requestedOpticalZoom() const
{
    return 0.0;
}

void AalCameraZoomControl::zoomTo(qreal optical, qreal digital)
{
    Q_UNUSED(optical);

    if (!m_service->androidControl())
        return;

    if (digital < 0.0 || digital > m_maximumDigitalZoom) {
        qWarning() << "Invalid zoom value:" << digital;
        return;
    }

    m_pendingZoom = static_cast<int>(digital);

    if (m_pendingZoom == m_currentDigitalZoom)
        return;

    android_camera_set_zoom(m_service->androidControl(), m_pendingZoom);
    m_currentDigitalZoom = m_pendingZoom;
    Q_EMIT currentDigitalZoomChanged(m_currentDigitalZoom);
}

void AalCameraZoomControl::init(CameraControl *control, CameraControlListener *listener)
{
    Q_UNUSED(control);
    Q_UNUSED(listener);

    resetZoom();
}

/*!
 * \brief AalCameraZoomControl::reset sets the current zoom value to 0 and the
 * maximum zoom value to the maximum zoom level that the hardware reports as
 * supporting
 */
void AalCameraZoomControl::resetZoom()
{
    if (!m_service->androidControl()) {
        return;
    }

    if (m_currentDigitalZoom != 0) {
        m_currentDigitalZoom = 0;
        Q_EMIT currentDigitalZoomChanged(m_currentDigitalZoom);
    }

    android_camera_set_zoom(m_service->androidControl(), m_currentDigitalZoom);

    int maxValue = 1;
    android_camera_get_max_zoom(m_service->androidControl(), &maxValue);
    if (maxValue < 0) {
        return;
    }

    if (maxValue != m_maximumDigitalZoom) {
        m_maximumDigitalZoom = maxValue;
        Q_EMIT maximumDigitalZoomChanged(m_maximumDigitalZoom);
    }
}
