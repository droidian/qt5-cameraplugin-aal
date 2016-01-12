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

AalCameraService *AalCameraService::m_service = 0;

AalCameraService::AalCameraService(QObject *parent) :
    QMediaService(parent),
    m_androidControl(0),
    m_androidListener(0)
{
    m_cameraControl = new AalCameraControl(this);
}

AalCameraService::~AalCameraService()
{
    delete m_cameraControl;
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
    return true;
}

void AalCameraService::disconnectCamera()
{
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

bool AalCameraService::isCameraActive() const
{
    return true;
}

void AalCameraService::onApplicationStateChanged()
{
}

void AalCameraService::initControls(CameraControl *camControl, CameraControlListener *listener)
{
    Q_UNUSED(camControl);
    Q_UNUSED(listener);
}

bool AalCameraService::isRecording() const
{
    return false;
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
