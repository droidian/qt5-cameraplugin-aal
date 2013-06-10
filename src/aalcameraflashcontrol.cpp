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
#include "aalcameraservice.h"

#include <QDebug>

#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>

AalCameraFlashControl::AalCameraFlashControl(AalCameraService *service, QObject *parent)
    : QCameraFlashControl(parent),
      m_service(service),
      m_currentMode(QCameraExposure::FlashManual),
      setOnInit(false)
{
}

QCameraExposure::FlashModes AalCameraFlashControl::flashMode() const
{
    return m_currentMode;
}

bool AalCameraFlashControl::isFlashModeSupported(QCameraExposure::FlashModes mode) const
{
    if (m_service->isBackCameraUsed()) {
        if (mode==QCameraExposure::FlashAuto || mode==QCameraExposure::FlashOff ||
                mode==QCameraExposure::FlashOn || mode==QCameraExposure::FlashVideoLight ||
                mode==QCameraExposure::FlashTorch) {
            return true;
        }
    } else {
        if (mode==QCameraExposure::FlashOff)
            return true;
    }

    return false;
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
    else {
        setOnInit = true;
    }
}

void AalCameraFlashControl::init(CameraControl *control)
{
    if (setOnInit) {
        FlashMode mode = qt2Android(m_currentMode);
        android_camera_set_flash_mode(control, mode);
        setOnInit = false;
    } else {
        FlashMode mode;
        android_camera_get_flash_mode(control, &mode);
        m_currentMode = android2Qt(mode);
    }

    if (!isFlashModeSupported(m_currentMode)) {
        setFlashMode(QCameraExposure::FlashOff);
    }

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
    case FLASH_MODE_OFF:
        return QCameraExposure::FlashOff;
    case FLASH_MODE_ON:
        return QCameraExposure::FlashOn;
    case FLASH_MODE_TORCH:
        return QCameraExposure::FlashTorch;
    case FLASH_MODE_AUTO:
    default:
        return QCameraExposure::FlashAuto;
    }
}
