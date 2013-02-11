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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AALCAMERAFLASHCONTROL_H
#define AALCAMERAFLASHCONTROL_H

#include <QCameraExposure>
#include <QCameraFlashControl>

#include "camera_compatibility_layer_capabilities.h"

class AalCameraService;
struct CameraControl;

class AalCameraFlashControl : public QCameraFlashControl
{
    Q_OBJECT
public:
    AalCameraFlashControl(AalCameraService *service, QObject *parent = 0);

    QCameraExposure::FlashModes flashMode() const;
    bool isFlashModeSupported(QCameraExposure::FlashModes mode) const;
    bool isFlashReady() const;
    void setFlashMode(QCameraExposure::FlashModes mode);

public Q_SLOTS:
    void init(CameraControl *control);

private:
    FlashMode qt2Android(QCameraExposure::FlashModes mode);
    QCameraExposure::FlashModes android2Qt(FlashMode mode);

    AalCameraService *m_service;
    QCameraExposure::FlashModes m_currentMode;
    bool setOnInit;
};

#endif // AALCAMERAFLASHCONTROL_H
