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

#include "aalcameracontrol.h"
#include "aalcameraflashcontrol.h"
#include "aalcamerafocuscontrol.h"
#include "aalcameraservice.h"
#include "aalcamerazoomcontrol.h"
#include "aalimagecapturecontrol.h"
#include "aalvideodeviceselectorcontrol.h"
#include "aalvideorenderercontrol.h"

#include "camera_compatibility_layer.h"

#include <QDebug>

AalCameraService *AalCameraService::m_service = 0;

AalCameraService::AalCameraService(QObject *parent):
    QMediaService(parent),
    m_androidControl(0),
    m_androidListener(0),
    m_oldAndroidControl(0)
{
    m_service = this;

    m_cameraControl = new AalCameraControl(this);
    m_flashControl = new AalCameraFlashControl(this);
    m_focusControl = new AalCameraFocusControl(this);
    m_zoomControl = new AalCameraZoomControl(this);
    m_imageCaptureControl = new AalImageCaptureControl(this);
    m_deviceSelectControl = new AalVideoDeviceSelectorControl(this);
    m_videoOutput = new AalVideoRendererControl(this);
}

AalCameraService::~AalCameraService()
{
    disconnectCamera();
    m_cameraControl->setState(QCamera::UnloadedState);
    delete m_cameraControl;
    delete m_flashControl;
    delete m_focusControl;
    delete m_zoomControl;
    delete m_imageCaptureControl;
    delete m_deviceSelectControl;
    delete m_videoOutput;
    if (m_oldAndroidControl)
        android_camera_delete(m_oldAndroidControl);
    if (m_androidControl)
        android_camera_delete(m_androidControl);
}

QMediaControl *AalCameraService::requestControl(const char *name)
{
    if (qstrcmp(name, QCameraControl_iid) == 0)
        return m_cameraControl;

    if (qstrcmp(name, QCameraFlashControl_iid) == 0)
        return m_flashControl;

    if (qstrcmp(name, QCameraFocusControl_iid) == 0)
        return m_focusControl;

    if (qstrcmp(name, QCameraImageCaptureControl_iid) == 0)
        return m_imageCaptureControl;

    if (qstrcmp(name, QCameraZoomControl_iid) == 0)
        return m_zoomControl;

    if (qstrcmp(name, QVideoDeviceSelectorControl_iid) == 0)
        return m_deviceSelectControl;

    if (qstrcmp(name, QVideoRendererControl_iid) == 0)
        return m_videoOutput;

    return 0;
}

void AalCameraService::releaseControl(QMediaControl *control)
{
    Q_UNUSED(control);
}

CameraControl *AalCameraService::androidControl()
{
    return m_androidControl;
}

bool AalCameraService::connectCamera()
{
    if (m_androidControl)
        return true;

    if (m_oldAndroidControl){
        /// FIXME
        /// becasue android_camera_disconnect() is asynchronous, it's not deleted directly when calling disconnect
        /// properly implemented, whe should be notified when it can be deleted
        /// in case 2 switches happend very fast, this delete might happen too eraly
        android_camera_delete(m_oldAndroidControl);
    }
    m_oldAndroidControl = m_androidControl;

    CameraType device = BACK_FACING_CAMERA_TYPE;
    if (m_deviceSelectControl->selectedDevice() == 1)
        device = FRONT_FACING_CAMERA_TYPE;

    m_androidListener = new CameraControlListener;
    memset(m_androidListener, 0, sizeof(*m_androidListener));

    m_androidControl = android_camera_connect_to(device, m_androidListener);
    if (!m_androidControl) {
        delete m_androidListener;
        m_androidListener = 0;
        return false;
    }

    m_androidListener->context = m_androidControl;
    initControls(m_androidControl, m_androidListener);
    m_videoOutput->startPreview();

    return true;
}

void AalCameraService::disconnectCamera()
{
    if (m_androidControl) {
        android_camera_disconnect(m_androidControl);
        m_androidControl = 0;
    }

    if (m_androidListener) {
        delete m_androidListener;
        m_androidListener = 0;
    }
}

void AalCameraService::initControls(CameraControl *camControl, CameraControlListener *listener)
{
    m_cameraControl->init(camControl, listener);
    m_imageCaptureControl->init(camControl, listener);
    m_flashControl->init(camControl);
    m_focusControl->init(camControl, listener);
    m_zoomControl->init(camControl, listener);
    m_videoOutput->init(camControl, listener);
}
