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

#include "aalcameraexposurecontrol.h"
#include "aalcameracontrol.h"
#include "aalcameraservice.h"

#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>

// Definition of this enum value is duplicated in camera-app
static const QCameraExposure::ExposureMode ExposureHdr = static_cast<QCameraExposure::ExposureMode>(QCameraExposure::ExposureModeVendor + 1);

AalCameraExposureControl::AalCameraExposureControl(AalCameraService *service, QObject *parent)
    : QCameraExposureControl(parent),
      m_service(service),
      m_requestedExposureMode(QCameraExposure::ExposureAuto),
      m_actualExposureMode(QCameraExposure::ExposureAuto)
{
    m_androidToQtExposureModes[SCENE_MODE_AUTO] = QCameraExposure::ExposureAuto;
    m_androidToQtExposureModes[SCENE_MODE_ACTION] = QCameraExposure::ExposureSports;
    m_androidToQtExposureModes[SCENE_MODE_NIGHT] = QCameraExposure::ExposureNight;
    m_androidToQtExposureModes[SCENE_MODE_PARTY] = QCameraExposure::ExposureParty;
    m_androidToQtExposureModes[SCENE_MODE_SUNSET] = QCameraExposure::ExposureSunset;
    m_androidToQtExposureModes[SCENE_MODE_HDR] = ExposureHdr;
}

void AalCameraExposureControl::init(CameraControl *control, CameraControlListener *listener)
{
    Q_UNUSED(listener);

    m_supportedExposureModes.clear();
    android_camera_enumerate_supported_scene_modes(control, &AalCameraExposureControl::supportedSceneModesCallback, this);

    setValue(QCameraExposureControl::ExposureMode, m_requestedExposureMode);

    Q_EMIT parameterRangeChanged(QCameraExposureControl::ExposureMode);
}

void AalCameraExposureControl::supportedSceneModesCallback(void *context, SceneMode sceneMode)
{
    AalCameraExposureControl *self = (AalCameraExposureControl*)context;
    self->m_supportedExposureModes << self->m_androidToQtExposureModes[sceneMode];
}

bool AalCameraExposureControl::setValue(ExposureParameter parameter, const QVariant& value)
{
    if (!value.isValid()) {
        return false;
    }

    if (parameter == QCameraExposureControl::ExposureMode) {
        if (value.value<QCameraExposure::ExposureMode>() != m_requestedExposureMode) {
            m_requestedExposureMode = value.value<QCameraExposure::ExposureMode>();
            Q_EMIT requestedValueChanged(QCameraExposureControl::ExposureMode);
        }

        if (m_service->androidControl() != NULL && m_supportedExposureModes.contains(m_requestedExposureMode)) {
            SceneMode sceneMode = m_androidToQtExposureModes.key(m_requestedExposureMode);
            android_camera_set_scene_mode(m_service->androidControl(), sceneMode);
            m_actualExposureMode = m_requestedExposureMode;
            Q_EMIT actualValueChanged(QCameraExposureControl::ExposureMode);
            return true;
        }
    }

    return false;
}

QVariant AalCameraExposureControl::requestedValue(ExposureParameter parameter) const
{
    if (parameter == QCameraExposureControl::ExposureMode) {
        return QVariant::fromValue(m_requestedExposureMode);
    }

    return QVariant();
}

QVariant AalCameraExposureControl::actualValue(ExposureParameter parameter) const
{
    if (parameter == QCameraExposureControl::ExposureMode) {
        return QVariant::fromValue(m_actualExposureMode);
    }

    return QVariant();
}

bool AalCameraExposureControl::isParameterSupported(ExposureParameter parameter) const
{
    switch (parameter) {
        case QCameraExposureControl::ISO:
            return false;
        case QCameraExposureControl::Aperture:
            return false;
        case QCameraExposureControl::ShutterSpeed:
            return false;
        case QCameraExposureControl::ExposureCompensation:
            return false;
        case QCameraExposureControl::FlashPower:
            return false;
        case QCameraExposureControl::FlashCompensation:
            return false;
        case QCameraExposureControl::TorchPower:
            return false;
        case QCameraExposureControl::SpotMeteringPoint:
            return false;
        case QCameraExposureControl::ExposureMode:
            return true;
        case QCameraExposureControl::MeteringMode:
            return false;
        default:
            return false;
    }
}

QVariantList AalCameraExposureControl::supportedParameterRange(ExposureParameter parameter, bool *continuous) const
{
    if (continuous != NULL) {
        *continuous = false;
    }

    if (parameter == QCameraExposureControl::ExposureMode) {
        QVariantList supported;
        Q_FOREACH(QCameraExposure::ExposureMode mode, m_supportedExposureModes) {
            supported << QVariant::fromValue(mode);
        }
        return supported;
    }

    return QVariantList();
}
