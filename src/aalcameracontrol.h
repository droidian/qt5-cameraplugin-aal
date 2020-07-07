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

#ifndef AALCAMERACONTROL_H
#define AALCAMERACONTROL_H

#include <QCameraControl>

class AalCameraService;
struct CameraControl;
struct CameraControlListener;

class AalCameraControl : public QCameraControl
{
Q_OBJECT
public:
    AalCameraControl(AalCameraService *service, QObject *parent = 0);
    ~AalCameraControl();

    QCamera::State state() const;
    void setState(QCamera::State state);

    QCamera::Status status() const;

    QCamera::CaptureModes captureMode() const;
    void setCaptureMode(QCamera::CaptureModes);
    bool isCaptureModeSupported(QCamera::CaptureModes mode) const;

    bool canChangeProperty(PropertyChangeType changeType, QCamera::Status status) const;

    static void errorCB(void* context);

public Q_SLOTS:
    void init(CameraControl *control, CameraControlListener *listener);

private Q_SLOTS:
    void handleError();

private:
    AalCameraService *m_service;
    QCamera::State m_state;
    QCamera::Status m_status;
    QCamera::CaptureModes m_captureMode;

    bool m_restoreStateWhenApplicationActive;
    QCamera::State m_cameraStateWhenApplicationActive;
    Qt::ApplicationState m_previousApplicationState;

    // Used as a slot but not declared as such to avoid problems with unit tests
    void onApplicationStateChanged();
    // Used to bypass m_restoreStateWhenApplicationActive
    void doSetState(QCamera::State state);

    friend AalCameraService;
    void setStatus(QCamera::Status);
};

#endif
