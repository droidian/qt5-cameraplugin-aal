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

#include "aalcameracontrol.h"
#include "aalcameraservice.h"
#include "aalvideorenderercontrol.h"

#include "camera_compatibility_layer.h"

#include <QDebug>

AalCameraControl::AalCameraControl(AalCameraService *service, QObject *parent)
   : QCameraControl(parent),
    m_service(service),
    m_control(0),
    m_state(QCamera::UnloadedState),
    m_status(QCamera::UnloadedStatus),
    m_captureMode(QCamera::CaptureStillImage)
{
}

AalCameraControl::~AalCameraControl()
{
    delete m_control;
}

QCamera::State AalCameraControl::state() const
{
    return m_state;
}

void AalCameraControl::setState(QCamera::State state)
{
    if (m_state == state)
        return;

    if (m_state == QCamera::UnloadedState) {
        m_control = android_camera_connect_to(FRONT_FACING_CAMERA_TYPE, m_service->listener());
        if (!m_control) {
            qWarning() << "Unable to connect to camera";
            return;
        }
        m_service->listener()->context = m_control;
        m_service->videoOutputControl()->startPreview();
    }

    qDebug() << m_state << "->" << state;
    m_state = state;
    Q_EMIT stateChanged(m_state);
}

QCamera::Status AalCameraControl::status() const
{
    return m_status;
}

QCamera::CaptureModes AalCameraControl::captureMode() const
{
    return m_captureMode;
}

void AalCameraControl::setCaptureMode(QCamera::CaptureModes mode)
{
    if (m_captureMode == mode)
        return;

    m_captureMode = mode;
    Q_EMIT captureModeChanged(mode);
}

bool AalCameraControl::isCaptureModeSupported(QCamera::CaptureModes mode) const
{
    return (QCamera::CaptureStillImage==mode) | (QCamera::CaptureVideo==mode);
}

bool AalCameraControl::canChangeProperty(QCameraControl::PropertyChangeType changeType, QCamera::Status status) const
{
    Q_UNUSED(changeType);
    Q_UNUSED(status);

    return true;
}

CameraControl *AalCameraControl::control()
{
    return m_control;
}
