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

#include "aalvideodeviceselectorcontrol.h"
#include "aalcameraservice.h"
#include "aalviewfindersettingscontrol.h"
#include "aalimageencodercontrol.h"

#include <QDebug>

#include "camera_compatibility_layer_capabilities.h"

AalVideoDeviceSelectorControl::AalVideoDeviceSelectorControl(AalCameraService *service, QObject *parent)
    :QVideoDeviceSelectorControl(parent),
      m_service(service),
      m_currentDevice(0),
      m_numberOfCameras(-1)
{
}

int AalVideoDeviceSelectorControl::defaultDevice() const
{
    return m_currentDevice;
}

int AalVideoDeviceSelectorControl::deviceCount() const
{
    if (m_numberOfCameras < 0)
        m_numberOfCameras = android_camera_get_number_of_devices();

    return m_numberOfCameras;
}

QString AalVideoDeviceSelectorControl::deviceDescription(int index) const
{
    switch (index) {
    case 0: return QLatin1String("Back camera");
    case 1: return QLatin1String("Front camera");
    default: return QLatin1String("");
    }
}

QString AalVideoDeviceSelectorControl::deviceName(int index) const
{
    switch (index) {
    case 0: return QLatin1String("Back");
    case 1: return QLatin1String("Front");
    default: return QLatin1String("");
    }
}

int AalVideoDeviceSelectorControl::selectedDevice() const
{
    return m_currentDevice;
}

void AalVideoDeviceSelectorControl::setSelectedDevice(int index)
{
    if (index == m_currentDevice)
        return;

    if (index < 0 || index >= deviceCount()) {
        qWarning() << "no valid device selected: " << index;
        return;
    }

    m_service->disconnectCamera();
    m_service->viewfinderControl()->resetAllSettings();
    m_service->imageEncoderControl()->resetAllSettings();
    m_currentDevice = index;
    if (m_service->isCameraActive())
        m_service->connectCamera();

    Q_EMIT selectedDeviceChanged(m_currentDevice);
    Q_EMIT selectedDeviceChanged(deviceName(m_currentDevice));
}
