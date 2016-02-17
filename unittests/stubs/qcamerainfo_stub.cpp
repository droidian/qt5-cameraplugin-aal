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

#include <QtMultimedia/QCameraInfo>
#include "qcamerainfodata.h"

class QCameraInfoPrivate {
public:
    QString deviceID;
    QString description;
    int orientation;
    QCamera::Position position;
};

QCameraInfo::QCameraInfo(const QByteArray &name) : d(new QCameraInfoPrivate())
{
    QStringList info = QString::fromLatin1(name).split('|');
    d->deviceID = info.at(0);
    d->description = info.at(1);
    d->orientation = info.at(2).toInt();
    d->position = QCamera::Position(info.at(3).toInt());
}

QCameraInfo::QCameraInfo(const QCamera &camera) : d(new QCameraInfoPrivate()) {
    Q_UNUSED(camera);
}

QCameraInfo::QCameraInfo(const QCameraInfo& other) : d(new QCameraInfoPrivate())
{
    d->deviceID = other.deviceName();
    d->description = other.description();
    d->orientation = other.orientation();
    d->position = other.position();
}
QCameraInfo::~QCameraInfo() { }

QCameraInfo& QCameraInfo::operator=(const QCameraInfo& other) {
    if (this != &other) {
        d->deviceID = other.deviceName();
        d->description = other.description();
        d->orientation = other.orientation();
        d->position = other.position();
    }
    return *this;
}
bool QCameraInfo::operator==(const QCameraInfo &other) const { Q_UNUSED(other); return false; }

bool QCameraInfo::isNull() const { return false; }

QString QCameraInfo::deviceName() const { return d->deviceID; }
QString QCameraInfo::description() const { return d->description; }
QCamera::Position QCameraInfo::position() const { return d->position; }
int QCameraInfo::orientation() const { return d->orientation; }

QCameraInfo QCameraInfo::defaultCamera() { return QCameraInfo(); }
QList<QCameraInfo> QCameraInfo::availableCameras(QCamera::Position position) {
    Q_UNUSED(position);

    QList<QCameraInfo> list;
    Q_FOREACH(CameraInfo info, QCameraInfoData::availableDevices) {
        QString infoString = QString("%1|%2|%3|%4").arg(info.deviceID).arg(info.description)
                                                   .arg(info.orientation).arg(info.position);
        QCameraInfo camera(infoString.toLatin1());
        list.append(camera);
    }

    return list;
}
