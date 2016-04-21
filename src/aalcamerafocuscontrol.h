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

#ifndef AALCAMERAFOCUSCONTROL_H
#define AALCAMERAFOCUSCONTROL_H

#include <QCameraFocusControl>

#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>

class AalCameraService;

class AalCameraFocusControl : public QCameraFocusControl
{
    Q_OBJECT
public:
    AalCameraFocusControl(AalCameraService *service, QObject *parent = 0);

    QPointF customFocusPoint() const;
    QCameraFocus::FocusModes focusMode() const;
    QCameraFocus::FocusPointMode focusPointMode() const;
    QCameraFocusZoneList focusZones() const;
    bool isFocusModeSupported(QCameraFocus::FocusModes mode) const;
    bool isFocusPointModeSupported(QCameraFocus::FocusPointMode mode) const;
    void setCustomFocusPoint(const QPointF & point);
    void setFocusMode(QCameraFocus::FocusModes mode);
    void setFocusPointMode(QCameraFocus::FocusPointMode mode);

    static void focusCB(void* context);

    bool isFocusBusy() const;

    void enablePhotoMode();
    void enableVideoMode();

public Q_SLOTS:
    void init(CameraControl *control, CameraControlListener *listener);
    void startFocus();

private:
    AutoFocusMode qt2Android(QCameraFocus::FocusModes mode);
    QCameraFocus::FocusModes android2Qt(AutoFocusMode mode);
    void point2Region(const QPointF &point, FocusRegion& region, MeteringRegion& metering) const;

    AalCameraService *m_service;
    QCameraFocus::FocusModes m_focusMode;
    QCameraFocus::FocusPointMode m_focusPointMode;
    QPointF m_focusPoint;
    FocusRegion m_focusRegion;
    bool m_focusRunning;
};

#endif // AALCAMERAFOCUSCONTROL_H
