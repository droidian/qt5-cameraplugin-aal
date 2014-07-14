/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#ifndef AALCAMERAEXPOSURECONTROL_H
#define AALCAMERAEXPOSURECONTROL_H

#include <QtCore/QMap>
#include <QCameraExposureControl>

#include <hybris/camera/camera_compatibility_layer_capabilities.h>

class AalCameraService;
class CameraControl;
class CameraControlListener;

class AalCameraExposureControl : public QCameraExposureControl
{
    Q_OBJECT
public:
    explicit AalCameraExposureControl(AalCameraService *service, QObject *parent = 0);

    void init(CameraControl *control, CameraControlListener *listener);
    bool setValue(ExposureParameter parameter, const QVariant& value);
    QVariant requestedValue(ExposureParameter parameter) const;
    QVariant actualValue(ExposureParameter parameter) const;
    bool isParameterSupported(ExposureParameter parameter) const;
    QVariantList supportedParameterRange(ExposureParameter parameter, bool *continuous) const;

    static void supportedSceneModesCallback(void *context, SceneMode sceneMode);

private:
    QMap<SceneMode, QCameraExposure::ExposureMode> m_androidToQtExposureModes;
    AalCameraService *m_service;
    QVariantList m_supportedExposureModes;
    QCameraExposure::ExposureMode m_requestedExposureMode;
    QCameraExposure::ExposureMode m_actualExposureMode;
};

#endif // AALCAMERAEXPOSURECONTROL_H
