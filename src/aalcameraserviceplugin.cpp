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

#include "aalcameraserviceplugin.h"
#include "aalcameraservice.h"

#include <QByteArray>
#include <QDebug>
#include <QMetaType>
#include <qgl.h>

#include <hybris/properties/properties.h>
#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>


AalServicePlugin::AalServicePlugin()
{
    // Devices are identified in android only by their index, so we do the same
    int cameras = android_camera_get_number_of_devices();
    for (int deviceId = 0; deviceId < cameras; deviceId++) {
        int facing;
        int orientation;
        int result = android_camera_get_device_info(deviceId, &facing, &orientation);
        if (result != 0 || facing < 0 || facing > 1 || orientation < 0 || orientation > 360) {
            qWarning() << "Failed to get camera info for device" << deviceId;
            continue;
        }   
        QString camera("%1");
        camera = camera.arg(deviceId);
        deviceList.append(camera.toLatin1());
        qWarning() << "Added camera" << camera;
    }
}

QMediaService* AalServicePlugin::create(QString const& key)
{
    if (key == QLatin1String(Q_MEDIASERVICE_CAMERA))
        return new AalCameraService;
    else
        qWarning() << "Key not supported:" << key;

    return 0;
}

void AalServicePlugin::release(QMediaService *service)
{
    delete service;
}

QList<QByteArray> AalServicePlugin::devices(const QByteArray &service) const
{

    if (deviceList.size() > 0 && QString::fromLatin1(service) == QLatin1String(Q_MEDIASERVICE_CAMERA)) {
        return deviceList;
    }
    
    return QList<QByteArray>();
}

QString AalServicePlugin::deviceDescription(const QByteArray &service, const QByteArray &device)
{
    if (QString::fromLatin1(service) != QLatin1String(Q_MEDIASERVICE_CAMERA)) {
        return QString();
    }

    // Android does not provice a descriptive identifier for devices, so we just
    // send back the index plus some useful human readable information about position.
    bool ok;
    int deviceID = device.toInt(&ok, 10);
    if (!ok || deviceID >= android_camera_get_number_of_devices()) {
        qWarning() << "Requested description for invalid device ID:" << device;
        return QString();
    } else {
        QCamera::Position position = cameraPosition(device);
        return QString("Camera %1%2").arg(QLatin1String(device))
                                     .arg(position == QCamera::FrontFace ? " Front facing" :
                                          (position == QCamera::BackFace ? " Back facing" : ""));
    }
}

int AalServicePlugin::getCameraOrientationOverride(const QString deviceID) const {
    QByteArray propertyName = QString("aal.camera.orientations.%1").arg(deviceID).toLocal8Bit();

    char orientationStr[PROP_VALUE_MAX];
    property_get(propertyName.data(), orientationStr, "");

    bool ok;
    int orientation = QString(orientationStr).toInt(&ok, /* base */ 10);
    if (!ok) {
        orientation = -1;
    }

    return orientation;
}

int AalServicePlugin::cameraOrientation(const QByteArray & device) const
{
    // Check for the override
    int override = getCameraOrientationOverride(device);
    if (override != -1) {
        return override;
    }

    int facing;
    int orientation;

    bool ok;
    int deviceID = device.toInt(&ok, 10);
    if (!ok) {
        return 0;
    }

    int result = android_camera_get_device_info(deviceID, &facing, &orientation);
    if (result != 0) {
        return 0;
    }

    // Android's orientation means differently compared to QT's orientation.
    // On Android, it means "the angle that the camera image needs to be
    // rotated", but on QT, it means "the physical orientation of the camera
    // sensor". So, the value will have to be inverted.

    return (360 - orientation) % 360;
}

QCamera::Position AalServicePlugin::cameraPosition(const QByteArray & device) const
{
    int facing;
    int orientation;

    bool ok;
    int deviceID = device.toInt(&ok, 10);
    if (!ok) {
        return QCamera::UnspecifiedPosition;
    }

    int result = android_camera_get_device_info(deviceID, &facing, &orientation);
    if (result != 0) {
        return QCamera::UnspecifiedPosition;
    } else {
        return facing == BACK_FACING_CAMERA_TYPE ? QCamera::BackFace :
                                                   QCamera::FrontFace;
    }
}
