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

RotationHandler::RotationHandler(AalCameraService *service, QObject *parent)
    : QObject(parent),
    m_deviceOrientation(0)
{
}

void RotationHandler::orientationChanged()
{
}

void RotationHandler::cameraStateChanged(QCamera::State state)
{
    Q_UNUSED(state);
}

int RotationHandler::calculateRotation()
{
    return m_deviceOrientation;
}
