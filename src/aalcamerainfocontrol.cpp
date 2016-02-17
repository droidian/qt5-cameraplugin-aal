/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#include "aalcamerainfocontrol.h"

#include <QCameraInfo>

AalCameraInfoControl::AalCameraInfoControl(QObject *parent) : QCameraInfoControl(parent)
{
}

QCamera::Position AalCameraInfoControl::cameraPosition(const QString &deviceName) const
{
    return QCameraInfo(deviceName.toLatin1()).position();
}

int AalCameraInfoControl::cameraOrientation(const QString &deviceName) const
{
    return QCameraInfo(deviceName.toLatin1()).orientation();
}
