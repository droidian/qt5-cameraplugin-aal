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

#include "aalcameraservice.h"
#include "aalcameraexposurecontrol.h"
#include "camera_control.h"
#include "camera_compatibility_layer.h"

AalCameraService *AalCameraService::m_service = 0;

AalCameraService::AalCameraService(QObject *parent) :
    QMediaService(parent),
    m_androidControl(0),
    m_androidListener(0)
{
    m_exposureControl = new AalCameraExposureControl(this);
}

AalCameraService::~AalCameraService()
{
    delete m_exposureControl;
}

QMediaControl *AalCameraService::requestControl(const char *name)
{
    Q_UNUSED(name);
    return 0;
}

void AalCameraService::releaseControl(QMediaControl *control)
{
    Q_UNUSED(control);
}

CameraControl *AalCameraService::androidControl()
{
    return m_androidControl;
}

bool AalCameraService::connectCamera()
{
    m_androidListener = new CameraControlListener;
    m_androidControl = android_camera_connect_to(BACK_FACING_CAMERA_TYPE, m_androidListener);

    initControls(m_androidControl, m_androidListener);

    return true;
}

void AalCameraService::disconnectCamera()
{
    delete m_androidListener;
    android_camera_disconnect(m_androidControl);
}

void AalCameraService::startPreview()
{
}

void AalCameraService::stopPreview()
{
}

void AalCameraService::initControls(CameraControl *camControl, CameraControlListener *listener)
{
    m_exposureControl->init(camControl, listener);
}

void AalCameraService::updateCaptureReady()
{
}
