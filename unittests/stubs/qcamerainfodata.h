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

#ifndef QCAMERAINFODATA_H
#define QCAMERAINFODATA_H

#include <QCamera>

class CameraInfo {
public:
    CameraInfo(QString deviceID = QString(), QString description = QString(),
               int orientation = 0, QCamera::Position position = QCamera::UnspecifiedPosition);

    QString deviceID;
    QString description;
    int orientation;
    QCamera::Position position;
};

class QCameraInfoData
{
public:
    static QList<CameraInfo> availableDevices;
};

#endif // QCAMERAINFODATA_H
