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

#include "aalcameraflashcontrol.h"
#include "aalcameracontrol.h"
#include "aalcameraservice.h"

#include <QDebug>

#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>

AalCameraFlashControl::AalCameraFlashControl(AalCameraService *service, QObject *parent)
    : QCameraFlashControl(parent),
      m_service(service),
      m_currentMode(QCameraExposure::FlashManual)
{
}

QCameraExposure::FlashModes AalCameraFlashControl::flashMode() const
{
    return m_currentMode;
}

bool AalCameraFlashControl::isFlashModeSupported(QCameraExposure::FlashModes mode) const
{
    return m_supportedModes.isEmpty() || m_supportedModes.contains(mode);
}

bool AalCameraFlashControl::isFlashReady() const
{
    return true;
}

void AalCameraFlashControl::setFlashMode(QCameraExposure::FlashModes mode)
{
    if (mode == m_currentMode || !isFlashModeSupported(mode))
        return;

    FlashMode fmode = qt2Android(mode);
    m_currentMode = mode;

    if (m_service->androidControl()) {
        android_camera_set_flash_mode(m_service->androidControl(), fmode);
    }
}

void AalCameraFlashControl::init(CameraControl *control)
{
    querySupportedFlashModes(control);

    FlashMode mode = qt2Android(m_currentMode);
    android_camera_set_flash_mode(control, mode);

    Q_EMIT flashReady(true);
}

FlashMode AalCameraFlashControl::qt2Android(QCameraExposure::FlashModes mode)
{
    switch(mode) {
    case QCameraExposure::FlashOff:
        return FLASH_MODE_OFF;
    case QCameraExposure::FlashOn:
        return FLASH_MODE_ON;
    case QCameraExposure::FlashVideoLight:
    case QCameraExposure::FlashTorch:
        return FLASH_MODE_TORCH;
    case QCameraExposure::FlashAuto:
    default:
        return FLASH_MODE_AUTO;
    }
}

QCameraExposure::FlashModes AalCameraFlashControl::android2Qt(FlashMode mode)
{
    switch(mode) {
    case FLASH_MODE_ON:
        return QCameraExposure::FlashOn;
    case FLASH_MODE_TORCH:
        return QCameraExposure::FlashVideoLight;
    case FLASH_MODE_AUTO:
        return QCameraExposure::FlashAuto;
    case FLASH_MODE_OFF:
    default:
        return QCameraExposure::FlashOff;
    }
}

/*!
 * \brief AalCameraFlashControl::querySupportedFlashModes gets the supported
 * flash modes for the current camera
 */
void AalCameraFlashControl::querySupportedFlashModes(CameraControl *control)
{
    m_supportedModes.clear();

    android_camera_enumerate_supported_flash_modes(control, &AalCameraFlashControl::supportedFlashModesCallback, this);
}

void AalCameraFlashControl::supportedFlashModesCallback(void *context, FlashMode flashMode)
{
    AalCameraFlashControl *self = (AalCameraFlashControl*)context;
    self->m_supportedModes << self->android2Qt(flashMode);
}

