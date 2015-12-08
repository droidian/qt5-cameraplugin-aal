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
#include "aalcameracontrol.h"
#include <aalcamerazoomcontrol.h>
#include <hybris/camera/camera_compatibility_layer.h>

AalCameraService *AalCameraService::m_service = 0;

AalCameraService::AalCameraService(QObject *parent) :
    QMediaService(parent),
    m_androidControl(0),
    m_androidListener(0)
{
    m_cameraControl = new AalCameraControl(this);
    m_zoomControl = new AalCameraZoomControl(this);
}

AalCameraService::~AalCameraService()
{
    delete m_cameraControl;
    delete m_zoomControl;
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

bool AalCameraService::isPreviewStarted() const
{
    return true;
}

void AalCameraService::onApplicationStateChanged()
{
}

void AalCameraService::initControls(CameraControl *camControl, CameraControlListener *listener)
{
    m_zoomControl->init(camControl, listener);
}

void AalCameraService::updateCaptureReady()
{
}

QSize AalCameraService::selectSizeWithAspectRatio(const QList<QSize> &sizes, float targetAspectRatio) const
{
    Q_UNUSED(sizes);
    Q_UNUSED(targetAspectRatio);
    return QSize();
}
