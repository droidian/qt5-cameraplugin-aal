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

#include "aalviewfindersettingscontrol.h"
#include "aalcameraservice.h"
#include "aalvideorenderercontrol.h"

#include <QDebug>

#include "camera_compatibility_layer_capabilities.h"

AalViewfinderSettingsControl::AalViewfinderSettingsControl(AalCameraService *service, QObject *parent)
    :QCameraViewfinderSettingsControl(parent),
      m_service(service),
      m_currentSize(),
      m_currentFPS(30),
      m_minFPS(10),
      m_maxFPS(30)
{
}

AalViewfinderSettingsControl::~AalViewfinderSettingsControl()
{
}

bool AalViewfinderSettingsControl::isViewfinderParameterSupported(ViewfinderParameter parameter) const
{
    if (parameter == QCameraViewfinderSettingsControl::Resolution ||
            parameter == QCameraViewfinderSettingsControl::MinimumFrameRate ||
            parameter == QCameraViewfinderSettingsControl::MaximumFrameRate ) {
        return true;
    }

    return false;
}

void AalViewfinderSettingsControl::setViewfinderParameter(ViewfinderParameter parameter, const QVariant &value)
{
    if (!isViewfinderParameterSupported(parameter)) {
        qWarning() << "Viewfinder dos not support parameter " << parameter;
        return;
    }

    switch (parameter) {
    case QCameraViewfinderSettingsControl::Resolution:
        setSize(value.toSize());
        break;
    case QCameraViewfinderSettingsControl::MinimumFrameRate:
    case QCameraViewfinderSettingsControl::MaximumFrameRate:
        qWarning() << "Camera framerate boundaries are set by the backend";
        break;
    default:
        break;
    }
}

QVariant AalViewfinderSettingsControl::viewfinderParameter(ViewfinderParameter parameter) const
{
    if (!isViewfinderParameterSupported(parameter)) {
        qWarning() << "Viewfinder dos not support parameter " << parameter;
        return QVariant();
    }

    switch (parameter) {
    case QCameraViewfinderSettingsControl::Resolution:
        return m_currentSize;
    case QCameraViewfinderSettingsControl::MinimumFrameRate:
        return m_minFPS;
    case QCameraViewfinderSettingsControl::MaximumFrameRate:
        return m_maxFPS;
    default:
        break;
    }

    return QVariant();
}

void AalViewfinderSettingsControl::setSize(const QSize &size)
{
    CameraControl *cc = m_service->androidControl();
    if (!cc) {
        m_currentSize = size; // will be used on next call of init
        return;
    }

    if (!m_availableSizes.contains(size)) {
        qWarning() << "Size " << size << "is not supported by the camera";
        qWarning() << "Supported sizes are: " << m_availableSizes;
        return;
    }

    m_currentSize = size;

    AalVideoRendererControl *videoRenderer = m_service->videoOutputControl();
    bool vfRunning = videoRenderer->isViewfinderRunning();

    if (vfRunning)
        videoRenderer->stopPreview();

    android_camera_set_preview_size(cc, m_currentSize.width(), m_currentSize.height());

    if (vfRunning)
        videoRenderer->startPreview();
}

QSize AalViewfinderSettingsControl::currentSize() const
{
    return m_currentSize;
}

void AalViewfinderSettingsControl::init(CameraControl *control, CameraControlListener *listener)
{
    Q_UNUSED(listener);

    if (m_availableSizes.isEmpty()) {
        android_camera_enumerate_supported_preview_sizes(control, &AalViewfinderSettingsControl::sizeCB, this);
    }
    if (m_currentSize.isEmpty()) {
        m_currentSize = m_availableSizes[1];
    } else {
        if (!m_availableSizes.contains(m_currentSize))
            m_currentSize = m_availableSizes[1];
    }
    android_camera_set_preview_size(control, m_currentSize.width(), m_currentSize.height());

    android_camera_get_preview_fps_range(control, &m_minFPS, &m_maxFPS);
    m_currentFPS = m_maxFPS;
    android_camera_set_preview_fps(control, m_currentFPS);
}

/*! Resets all data, so a new init starts with a fresh start
    This is used when switching the cameras
    Only works when there is no camera active/connected
*/
void AalViewfinderSettingsControl::resetAllSettings()
{
    if (m_service->androidControl())
        return;

    m_currentSize = QSize();
    m_availableSizes.clear();

    m_currentFPS = 0;
    m_minFPS = 0;
    m_maxFPS = 0;
}

void AalViewfinderSettingsControl::sizeCB(void *ctx, int width, int height)
{
    AalViewfinderSettingsControl *self = (AalViewfinderSettingsControl*)ctx;
    self->m_availableSizes.append(QSize(width, height));
}
