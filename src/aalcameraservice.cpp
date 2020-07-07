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
#include "aalcamerainfocontrol.h"
#include "storagemanager.h"
#include "aalcameraexposurecontrol.h"
#include "rotationhandler.h"

#include <hybris/camera/camera_compatibility_layer.h>

#include <QDebug>
#include <cmath>

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
    m_infoControl = new AalCameraInfoControl(this);
    m_rotationHandler = new RotationHandler(this);
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
    delete m_infoControl;
    if (m_androidControl)
        android_camera_delete(m_androidControl);
    delete m_storageManager;
    delete m_rotationHandler;
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

    if (qstrcmp(name, QCameraInfoControl_iid) == 0)
        return m_infoControl;

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

RotationHandler *AalCameraService::rotationHandler()
{
    return m_rotationHandler;
}

bool AalCameraService::connectCamera()
{
    if (m_androidControl)
        return true;

    m_androidListener = new CameraControlListener;
    memset(m_androidListener, 0, sizeof(*m_androidListener));

    // if there is only one camera fallback directly to the ID of whatever device we have
    if (m_deviceSelectControl->deviceCount() == 1) {
        m_androidControl = android_camera_connect_by_id(m_deviceSelectControl->selectedDevice(), m_androidListener);
    } else {
        CameraType device = BACK_FACING_CAMERA_TYPE;
        if (!isBackCameraUsed()) {
            device = FRONT_FACING_CAMERA_TYPE;
        }

        m_androidControl = android_camera_connect_to(device, m_androidListener);
    }

    if (!m_androidControl) {
        delete m_androidListener;
        m_androidListener = 0;
        return false;
    }

    m_androidListener->context = m_androidControl;
    initControls(m_androidControl, m_androidListener);

    this->m_cameraControl->setStatus(QCamera::LoadedStatus);

    return true;
}

void AalCameraService::disconnectCamera()
{
    if (m_imageCaptureControl->isCaptureRunning()) {
        m_imageCaptureControl->cancelCapture();
    }

    stopPreview();

    if (m_androidControl) {
        android_camera_disconnect(m_androidControl);
        m_androidControl = 0;
    }

    if (m_androidListener) {
        delete m_androidListener;
        m_androidListener = 0;
    }

    this->m_cameraControl->setStatus(QCamera::UnloadedStatus);
}

void AalCameraService::startPreview()
{
    if (m_videoOutput) {
        m_videoOutput->startPreview();
    }

    this->m_cameraControl->setStatus(QCamera::ActiveStatus);
}

void AalCameraService::stopPreview()
{
    if (m_videoOutput) {
        m_videoOutput->stopPreview();
    }

    this->m_cameraControl->setStatus(QCamera::LoadedStatus);
}

bool AalCameraService::isPreviewStarted() const
{
    if (m_videoOutput) {
        return m_videoOutput->isPreviewStarted();
    } else {
        return false;
    }
}

bool AalCameraService::isCameraActive() const
{
    return m_cameraControl->state() == QCamera::ActiveState;
}

bool AalCameraService::isBackCameraUsed() const
{
    int deviceIndex = m_deviceSelectControl->selectedDevice();
    QString deviceName = m_deviceSelectControl->deviceName(deviceIndex);
    return m_infoControl->cameraPosition(deviceName) == QCamera::BackFace;
}

/*!
 * \brief AalCameraService::enablePhotoMode sets all controls into photo mode
 */
void AalCameraService::enablePhotoMode()
{
    if (isPreviewStarted())
        // Trick to make applications notice the change.
        this->m_cameraControl->setStatus(QCamera::StartingStatus);

    m_flashControl->init(m_service->androidControl());
    m_imageEncoderControl->enablePhotoMode();
    m_focusControl->enablePhotoMode();
    m_viewfinderControl->setAspectRatio(m_imageEncoderControl->getAspectRatio());

    if (isPreviewStarted())
        this->m_cameraControl->setStatus(QCamera::ActiveStatus);
}

/*!
 * \brief AalCameraService::enableVideoMode sets all controls into video mode
 */
void AalCameraService::enableVideoMode()
{
    if (isPreviewStarted())
        // Trick to make applications notice the change.
        this->m_cameraControl->setStatus(QCamera::StartingStatus);

    m_flashControl->init(m_service->androidControl());
    m_focusControl->enableVideoMode();
    m_viewfinderControl->setAspectRatio(m_videoEncoderControl->getAspectRatio());

    if (isPreviewStarted())
        this->m_cameraControl->setStatus(QCamera::ActiveStatus);
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
    if (!isPreviewStarted())
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
    m_videoOutput->init(camControl, listener);
    m_viewfinderControl->init(camControl, listener);
    m_imageEncoderControl->init(camControl);
    m_imageCaptureControl->init(camControl, listener);
    m_flashControl->init(camControl);
    m_focusControl->init(camControl, listener);
    m_zoomControl->init(camControl, listener);
    m_videoEncoderControl->init(camControl, listener);
    m_exposureControl->init(camControl, listener);
}

QSize AalCameraService::selectSizeWithAspectRatio(const QList<QSize> &sizes, float targetAspectRatio) const
{
    QSize selectedSize;
    long selectedPixelCount = 0;
    const float EPSILON = 0.02;

    if (!sizes.empty()) {
        // Loop over all sizes until we find the highest one that matches targetAspectRatio.
        QList<QSize>::const_iterator it = sizes.begin();
        while (it != sizes.end()) {
            QSize size = *it;
            const float aspectRatio = (float)size.width() / (float)size.height();
            const long pixelCount = (long)size.width() * (long)size.height();
            if (fabs(aspectRatio - targetAspectRatio) < EPSILON && pixelCount > selectedPixelCount) {
                selectedSize = size;
                selectedPixelCount = pixelCount;
            }
            ++it;
        }
    }

    return selectedSize;
}
