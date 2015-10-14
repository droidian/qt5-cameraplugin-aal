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

#include "aalviewfindersettingscontrol.h"
#include "aalcameraservice.h"
#include "aalvideorenderercontrol.h"

#include <QDebug>

#include <cmath>

#include <hybris/camera/camera_compatibility_layer_capabilities.h>

AalViewfinderSettingsControl::AalViewfinderSettingsControl(AalCameraService *service, QObject *parent)
    :QCameraViewfinderSettingsControl(parent),
      m_service(service),
      m_currentSize(),
      m_aspectRatio(0.0),
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
    qWarning() << "AalViewfinderSettingsControl::setSize:" << size;
    if (size == m_currentSize)
        return;

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
    qWarning() << "AalViewfinderSettingsControl::setSize: GOING FOR IT" << size;

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

/*!
 * \brief AalViewfinderSettingsControl::supportedSizes returns the supported viewfinder
 * sizes
 * \return
 */
const QList<QSize> &AalViewfinderSettingsControl::supportedSizes() const
{
    if (m_availableSizes.isEmpty()) {
        CameraControl *cc = m_service->androidControl();
        if (cc) {
            AalViewfinderSettingsControl *vfControl = const_cast<AalViewfinderSettingsControl*>(this);
            android_camera_enumerate_supported_preview_sizes(cc,
                                                             &AalViewfinderSettingsControl::sizeCB,
                                                             vfControl);
        }
    }

    return m_availableSizes;
}

/*!
 * \brief AalViewfinderSettingsControl::setAspectRatio sets the viewfinder's aspect ratio
 * \param ratio the aspect ratio that should be used
 */
void AalViewfinderSettingsControl::setAspectRatio(float ratio)
{
    if (ratio == m_aspectRatio)
        return;

    m_aspectRatio = ratio;

    // Choose optimal resolution based on the current camera's aspect ratio
    QSize size = chooseOptimalSize(m_availableSizes);
    qDebug() << "AalViewfinderSettingsControl::setAspectRatio" << ratio << size;
    setSize(size);
}

void AalViewfinderSettingsControl::init(CameraControl *control, CameraControlListener *listener)
{
    Q_UNUSED(listener);

    if (m_availableSizes.isEmpty()) {
        android_camera_enumerate_supported_preview_sizes(control, &AalViewfinderSettingsControl::sizeCB, this);
    }

    // Choose optimal resolution based on the current camera's aspect ratio
    if (m_currentSize.isEmpty()) {
        m_currentSize = chooseOptimalSize(m_availableSizes);
    }
    android_camera_set_preview_size(control, m_currentSize.width(), m_currentSize.height());

    android_camera_get_preview_fps_range(control, &m_minFPS, &m_maxFPS);
    m_minFPS /= 1000;
    m_maxFPS /= 1000;
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

QSize AalViewfinderSettingsControl::chooseOptimalSize(const QList<QSize> &sizes) const
{
    qDebug() << "AalViewfinderSettingsControl:: CHOOOOSE OPTIMAL" << sizes << m_aspectRatio;
    if (!sizes.empty()) {
        if (m_aspectRatio == 0) {
            // There are resolutions supported, choose one non-optimal one):
            return sizes[1];
        }

        QSize largestSize;
        QList<QSize>::const_iterator it = sizes.begin();
        while (it != sizes.end()) {
            QSize size = *it;
            const float ratio = (float)size.width() / (float)size.height();
            const float EPSILON = 0.02;
            if (fabs(ratio - m_aspectRatio) < EPSILON) {
                if (size.width() * size.height() > largestSize.width() * largestSize.height()) {
                    largestSize = size;
                }
            }
            ++it;
        }
        return largestSize;
    }

    return QSize();
}
