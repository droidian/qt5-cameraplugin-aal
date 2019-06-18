/*
 * Copyright (C) 2019 Ratchanan Srirattanamet
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

#include "rotationhandler.h"

#include <QCameraInfo>
#include <QOrientationReading>

#include "aalcameracontrol.h"
#include "aalvideodeviceselectorcontrol.h"

RotationHandler::RotationHandler(AalCameraService *service, QObject *parent):
    QObject(parent),
    m_orientationSensor(this),
    m_service(service),
    m_deviceOrientation(0)
{
    connect(&m_orientationSensor, SIGNAL(readingChanged()), this, SLOT(orientationChanged()));
    connect(service->cameraControl(), SIGNAL(stateChanged(QCamera::State)),
                                this, SLOT(cameraStateChanged(QCamera::State)));
}

void RotationHandler::orientationChanged()
{
    switch (m_orientationSensor.reading()->orientation()) {
        case QOrientationReading::Orientation::TopUp:
            m_deviceOrientation = 0;
            break;

        case QOrientationReading::Orientation::LeftUp:
            m_deviceOrientation = 90;
            break;

        case QOrientationReading::Orientation::TopDown:
            m_deviceOrientation = 180;
            break;

        case QOrientationReading::Orientation::RightUp:
            m_deviceOrientation = 270;
            break;

        default:
            // Retain the last value.
            break;
    }
}

void RotationHandler::cameraStateChanged(QCamera::State state)
{
    // Listen to orientation change only if we're active.
    if (state == QCamera::ActiveState) {
        m_orientationSensor.start();
    } else {
        m_orientationSensor.stop();
    }
}

int RotationHandler::calculateRotation()
{
    int selectedDevice = m_service->deviceSelector()->selectedDevice();
    QCameraInfo cameraInfo = QCameraInfo::availableCameras().value(selectedDevice);

    // Starts of by getting device orientation
    int rotation = m_deviceOrientation;

    if (cameraInfo.position() == QCamera::FrontFace) {
        // Clockwise device becomes counter-clockwise camera
        rotation = (360 - rotation);
    }

    // Account for camera orientation
    rotation -= cameraInfo.orientation();

    // Ensure rotation is positive
    rotation = (rotation + 360) % 360;

    return rotation;
}
