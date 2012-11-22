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

#include "aalimagecapturecontrol.h"

#include "camera_compatibility_layer.h"
#include "camera_compatibility_layer_capabilities.h"

StorageManager::StorageManager()
{
}

AalImageCaptureControl::AalImageCaptureControl(AalCameraService *service, QObject *parent)
    : QCameraImageCaptureControl(parent)
{
    Q_UNUSED(service);
}

AalImageCaptureControl::~AalImageCaptureControl()
{
}

int AalImageCaptureControl::capture(const QString &fileName)
{
    Q_UNUSED(fileName);
    return 0;
}

void AalImageCaptureControl::cancelCapture()
{
}

bool AalImageCaptureControl::isReadyForCapture() const
{
    return true;
}

void AalImageCaptureControl::init(CameraControl *control, CameraControlListener *listener)
{
    Q_UNUSED(control);
    Q_UNUSED(listener);
}

void AalImageCaptureControl::setReady(bool ready)
{
    Q_UNUSED(ready);
}

void AalImageCaptureControl::shutter()
{
}
