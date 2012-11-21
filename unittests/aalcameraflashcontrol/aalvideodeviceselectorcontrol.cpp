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

#include "aalvideodeviceselectorcontrol.h"
//#include "aalcameraservice.h"

#include "camera_compatibility_layer.h"
#include "camera_compatibility_layer_capabilities.h"

AalVideoDeviceSelectorControl::AalVideoDeviceSelectorControl(AalCameraService *service, QObject *parent)
    : QVideoDeviceSelectorControl(parent),
      m_currentDevice(0)
{
    Q_UNUSED(service);
}

int AalVideoDeviceSelectorControl::defaultDevice() const
{
    return 0;
}

int AalVideoDeviceSelectorControl::deviceCount() const
{
    return 2;
}

QString AalVideoDeviceSelectorControl::deviceDescription(int index) const
{
    Q_UNUSED(index);
    return QString();
}

QString AalVideoDeviceSelectorControl::deviceName(int index) const
{
    Q_UNUSED(index);
    return QString();
}

int AalVideoDeviceSelectorControl::selectedDevice() const
{
    return m_currentDevice;
}

void AalVideoDeviceSelectorControl::setSelectedDevice(int index)
{
    m_currentDevice = index;
}
