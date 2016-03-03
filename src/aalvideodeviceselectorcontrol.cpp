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
#include "aalcameracontrol.h"
#include "aalimageencodercontrol.h"
#include "aalvideoencodersettingscontrol.h"
#include "aalviewfindersettingscontrol.h"

#include <QDebug>
#include <QtMultimedia/QCamera>
#include <QtMultimedia/QCameraInfo>

#include <hybris/camera/camera_compatibility_layer_capabilities.h>

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
    return QCameraInfo::availableCameras().count();
}

QString AalVideoDeviceSelectorControl::deviceDescription(int index) const
{
    return QCameraInfo::availableCameras().value(index).description();
}

QString AalVideoDeviceSelectorControl::deviceName(int index) const
{
    return QCameraInfo::availableCameras().value(index).deviceName();
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

    if (m_service->isRecording())
        return;

    m_service->stopPreview();
    m_service->disconnectCamera();
    m_service->viewfinderControl()->resetAllSettings();
    m_service->imageEncoderControl()->resetAllSettings();
    m_service->videoEncoderControl()->resetAllSettings();
    m_currentDevice = index;
    QCamera::State state = m_service->cameraControl()->state();
    if (state == QCamera::LoadedState) {
        m_service->connectCamera();
    } else if (state == QCamera::ActiveState) {
        m_service->connectCamera();
        m_service->startPreview();
    }

    Q_EMIT selectedDeviceChanged(m_currentDevice);
    Q_EMIT selectedDeviceChanged(deviceName(m_currentDevice));
}
