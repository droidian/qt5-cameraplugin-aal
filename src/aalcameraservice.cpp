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

#include "aalcameracontrol.h"
#include "aalcameraflashcontrol.h"
#include "aalcamerafocuscontrol.h"
#include "aalcameraservice.h"
#include "aalcamerazoomcontrol.h"
#include "aalimagecapturecontrol.h"
#include "aalimageencodercontrol.h"
#include "aalvideodeviceselectorcontrol.h"
#include "aalvideorenderercontrol.h"
#include "aalviewfindersettingscontrol.h"

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
    m_imageEncoderControl = new AalImageEncoderControl(this);
    m_deviceSelectControl = new AalVideoDeviceSelectorControl(this);
    m_videoOutput = new AalVideoRendererControl(this);
    m_viewfinderControl = new AalViewfinderSettingsControl(this);
}

AalCameraService::~AalCameraService()
{
    disconnectCamera();
    m_cameraControl->setState(QCamera::UnloadedState);
    delete m_cameraControl;
    delete m_flashControl;
    delete m_focusControl;
    delete m_zoomControl;
    delete m_imageEncoderControl;
    delete m_imageCaptureControl;
    delete m_deviceSelectControl;
    delete m_videoOutput;
    delete m_viewfinderControl;
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

    if (qstrcmp(name, QImageEncoderControl_iid) == 0)
        return m_imageEncoderControl;

    if (qstrcmp(name, QCameraZoomControl_iid) == 0)
        return m_zoomControl;

    if (qstrcmp(name, QVideoDeviceSelectorControl_iid) == 0)
        return m_deviceSelectControl;

    if (qstrcmp(name, QVideoRendererControl_iid) == 0)
        return m_videoOutput;

    if (qstrcmp(name, QCameraViewfinderSettingsControl_iid) == 0)
        return m_viewfinderControl;

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
    if (!isBackCameraUsed())
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
    if (m_service->videoOutputControl()->isViewfinderRunning())
        m_service->videoOutputControl()->stopPreview();

    if (m_androidControl) {
        android_camera_disconnect(m_androidControl);
        m_androidControl = 0;
    }

    if (m_androidListener) {
        delete m_androidListener;
        m_androidListener = 0;
    }
}

bool AalCameraService::isCameraActive() const
{
    return m_cameraControl->state() == QCamera::ActiveState;
}

bool AalCameraService::isBackCameraUsed() const
{
    return m_deviceSelectControl->selectedDevice() == 0;
}

void AalCameraService::updateCaptureReady()
{
    bool ready = true;

    if (!(m_cameraControl->state() == QCamera::ActiveState))
        ready = false;
    if (m_imageCaptureControl->isCaptureRunning())
        ready = false;
    if (m_focusControl->isFocusBusy())
        ready = false;
    if (!m_videoOutput->isViewfinderRunning())
        ready = false;

    m_imageCaptureControl->setReady(ready);
}

void AalCameraService::initControls(CameraControl *camControl, CameraControlListener *listener)
{
    m_cameraControl->init(camControl, listener);
    m_imageEncoderControl->init(camControl);
    m_imageCaptureControl->init(camControl, listener);
    m_flashControl->init(camControl);
    m_focusControl->init(camControl, listener);
    m_zoomControl->init(camControl, listener);
    m_viewfinderControl->setAspectRatio(m_imageCaptureControl->getAspectRatio());
    m_viewfinderControl->init(camControl, listener);
    m_videoOutput->init(camControl, listener);
}
