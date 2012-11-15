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

#ifndef AALVIDEODEVICESELECTORCONTROL_H
#define AALVIDEODEVICESELECTORCONTROL_H

#include <QVideoDeviceSelectorControl>

class AalCameraService;

class AalVideoDeviceSelectorControl : public QVideoDeviceSelectorControl
{
public:
    AalVideoDeviceSelectorControl(AalCameraService *service, QObject *parent = 0);

    int defaultDevice() const;
    int deviceCount() const;
    QString deviceDescription(int index) const;
    QString deviceName(int index) const;
    int selectedDevice() const;

public Q_SLOTS:
    void setSelectedDevice(int index);

private:
    AalCameraService *m_service;
    int m_currentDevice;
    mutable int m_numberOfCameras;
};

#endif // AALVIDEODEVICESELECTORCONTROL_H
