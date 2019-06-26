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

#ifndef ROTATIONHANDLER_H
#define ROTATIONHANDLER_H

#include <QObject>
#include <QCamera>
#include <QOrientationSensor>

#include "aalcameraservice.h"

class RotationHandler: public QObject {
    Q_OBJECT

public:
    explicit RotationHandler(AalCameraService *service, QObject *parent = 0);

    int calculateRotation();

public Q_SLOTS:
    void orientationChanged();
    void cameraStateChanged(QCamera::State state);

private:
    QOrientationSensor m_orientationSensor;
    AalCameraService *m_service;
    int m_deviceOrientation;
};

#endif
