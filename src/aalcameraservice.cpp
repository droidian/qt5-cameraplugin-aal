/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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
#include "aalmediarecordercontrol.h"
#include "aalmetadatawritercontrol.h"
#include "aalvideodeviceselectorcontrol.h"
#include "aalvideoencodersettingscontrol.h"
#include "aalvideorenderercontrol.h"
#include "aalviewfindersettingscontrol.h"
#include "storagemanager.h"
#include "aalcameraexposurecontrol.h"

#include <hybris/camera/camera_compatibility_layer.h>

#include <QDebug>

AalCameraService *AalCameraService::m_service = 0;

AalCameraService::AalCameraService(QObject *parent):
    QMediaService(parent),
    m_androidControl(0),
    m_androidListener(0)
{
    m_service = this;

    m_storageManager = new StorageManager;
    m_cameraControl = new AalCameraControl(this);
    m_flashControl = new AalCameraFlashControl(this);
    m_focusControl = new AalCameraFocusControl(this);
    m_zoomControl = new AalCameraZoomControl(this);
    m_imageCaptureControl = new AalImageCaptureControl(this);
    m_imageEncoderControl = new AalImageEncoderControl(this);
    m_mediaRecorderControl = new AalMediaRecorderControl(this);
    m_metadataWriter = new AalMetaDataWriterControl(this);
    m_deviceSelectControl = new AalVideoDeviceSelectorControl(this);
    m_videoEncoderControl = new AalVideoEncoderSettingsControl(this);
    m_videoOutput = new AalVideoRendererControl(this);
    m_viewfinderControl = new AalViewfinderSettingsControl(this);
    m_exposureControl = new AalCameraExposureControl(this);
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
    delete m_mediaRecorderControl;
    delete m_metadataWriter;
    delete m_deviceSelectControl;
    delete m_videoEncoderControl;
    delete m_videoOutput;
    delete m_viewfinderControl;
    delete m_exposureControl;
    if (m_androidControl)
        android_camera_delete(m_androidControl);
    delete m_storageManager;
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

    if (qstrcmp(name, QMediaRecorderControl_iid) == 0)
        return m_mediaRecorderControl;

    if (qstrcmp(name, QMetaDataWriterControl_iid) == 0)
        return m_metadataWriter;

    if (qstrcmp(name, QCameraZoomControl_iid) == 0)
        return m_zoomControl;

    if (qstrcmp(name, QVideoDeviceSelectorControl_iid) == 0)
        return m_deviceSelectControl;

    if (qstrcmp(name, QVideoEncoderSettingsControl_iid) == 0)
        return m_videoEncoderControl;

    if (qstrcmp(name, QVideoRendererControl_iid) == 0)
        return m_videoOutput;

    if (qstrcmp(name, QCameraViewfinderSettingsControl_iid) == 0)
        return m_viewfinderControl;

    if (qstrcmp(name, QCameraExposureControl_iid) == 0)
        return m_exposureControl;

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

StorageManager *AalCameraService::storageManager()
{
    return m_storageManager;
}

bool AalCameraService::connectCamera()
{
    qDebug() << "CONNECT CAMERA" << m_androidControl;

    if (m_androidControl)
        return true;

    CameraType device = BACK_FACING_CAMERA_TYPE;
    if (!isBackCameraUsed())
        device = FRONT_FACING_CAMERA_TYPE;

    m_androidListener = new CameraControlListener;
    memset(m_androidListener, 0, sizeof(*m_androidListener));

    m_androidControl = android_camera_connect_to(device, m_androidListener);

    // fallback if there is only one camera
    if (!m_androidControl && m_deviceSelectControl->deviceCount() == 1) {
        if (device == BACK_FACING_CAMERA_TYPE)
            device = FRONT_FACING_CAMERA_TYPE;
        else
            device = BACK_FACING_CAMERA_TYPE;
        m_androidControl = android_camera_connect_to(device, m_androidListener);
    }

    if (!m_androidControl) {
        delete m_androidListener;
        m_androidListener = 0;
        return false;
    }

    m_androidListener->context = m_androidControl;
    initControls(m_androidControl, m_androidListener);

    return true;
}

void AalCameraService::disconnectCamera()
{
    qDebug() << "DIS-CONNECT CAMERA" << m_androidControl;
    stopPreview();

    if (m_androidControl) {
        android_camera_disconnect(m_androidControl);
        m_androidControl = 0;
    }

    if (m_androidListener) {
        delete m_androidListener;
        m_androidListener = 0;
    }
}

void AalCameraService::startPreview()
{
    qDebug() << "START PREVIEW" << m_androidControl << m_videoOutput;
    if (m_videoOutput) {
        qDebug() << "STARTING PREVIEW";
        m_videoOutput->startPreview();
        qDebug() << "STARTED PREVIEW";
    }
}

void AalCameraService::stopPreview()
{
    qDebug() << "STOP PREVIEW" << m_androidControl << m_videoOutput;
    if (m_videoOutput) {
        qDebug() << "STOPPING PREVIEW";
        m_videoOutput->stopPreview();
        qDebug() << "STOPPED PREVIEW";
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

/*!
 * \brief AalCameraService::enablePhotoMode sets all controls into photo mode
 */
void AalCameraService::enablePhotoMode()
{
    m_imageEncoderControl->enablePhotoMode();
    m_focusControl->enablePhotoMode();
    m_zoomControl->enablePhotoMode();
    m_viewfinderControl->setAspectRatio(m_imageEncoderControl->getAspectRatio());
}

/*!
 * \brief AalCameraService::enableVideoMode sets all controls into video mode
 */
void AalCameraService::enableVideoMode()
{
    m_focusControl->enableVideoMode();
    m_zoomControl->enableVideoMode();
    m_viewfinderControl->setAspectRatio(m_videoEncoderControl->getAspectRatio());
}

/*!
 * \brief AalCameraService::isRecording returns true is a video recording is
 * currently ongoing
 * \return
 */
bool AalCameraService::isRecording() const
{
    return m_mediaRecorderControl->state() != QMediaRecorder::StoppedState;
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

/*!
 * \brief AalCameraService::initControls initialize all the controls for a newly
 * connected camera
 * \param camControl
 * \param listener
 */
void AalCameraService::initControls(CameraControl *camControl, CameraControlListener *listener)
{
    m_cameraControl->init(camControl, listener);
    m_imageEncoderControl->init(camControl);
    m_imageCaptureControl->init(camControl, listener);
    m_flashControl->init(camControl);
    m_focusControl->init(camControl, listener);
    m_zoomControl->init(camControl, listener);
    m_videoEncoderControl->init(camControl, listener);
    qDebug() << "INIT CONTROLS" << m_imageEncoderControl->getAspectRatio();

    m_viewfinderControl->init(camControl, listener);
    if (m_cameraControl->captureMode() == QCamera::CaptureStillImage)
        m_viewfinderControl->setAspectRatio(m_imageEncoderControl->getAspectRatio());
    else
        m_viewfinderControl->setAspectRatio(m_videoEncoderControl->getAspectRatio());
    m_videoOutput->init(camControl, listener);
    m_exposureControl->init(camControl, listener);
}
