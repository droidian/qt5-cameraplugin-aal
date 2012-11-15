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

#include "aalimagecapturecontrol.h"
#include "aalcameracontrol.h"
#include "aalcamerafocuscontrol.h"
#include "aalcameraservice.h"
#include "aalvideodeviceselectorcontrol.h"
#include "aalvideorenderercontrol.h"
#include "storagemanager.h"

#include <camera_compatibility_layer.h>
#include <camera_compatibility_layer_capabilities.h>

#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>

AalImageCaptureControl::AalImageCaptureControl(AalCameraService *service, QObject *parent)
   : QCameraImageCaptureControl(parent),
    m_service(service),
    m_cameraControl(service->cameraControl()),
    m_lastRequestId(0),
    m_ready(false),
    m_pendingCaptureFile(),
    m_photoWidth(320),
    m_photoHeight(240)
{
    QObject::connect(m_cameraControl, SIGNAL(stateChanged(QCamera::State)),
                     this, SLOT(updateReady()));
}

AalImageCaptureControl::~AalImageCaptureControl()
{
}

bool AalImageCaptureControl::isReadyForCapture() const
{
    return m_ready;
}

int AalImageCaptureControl::capture(const QString &fileName)
{
    if (!m_ready || !m_service->androidControl()) {
        qWarning() << "Camera not ready to capture";
        return -1;
    }

    m_lastRequestId++;

    QFileInfo fi(fileName);
    if (fileName.isEmpty() || fi.isDir()) {
        m_pendingCaptureFile = m_storageManager.nextPhotoFileName(fileName);
    } else {
        m_pendingCaptureFile = fileName;
    }
    m_storageManager.checkDirectory(m_pendingCaptureFile);

    android_camera_take_snapshot(m_service->androidControl());

    updateReady();
    return m_lastRequestId;
}

void AalImageCaptureControl::cancelCapture()
{
}

void AalImageCaptureControl::shutterCB(void *context)
{
    Q_UNUSED(context);
    QMetaObject::invokeMethod(AalCameraService::instance()->imageCaptureControl(),
                              "shutter", Qt::QueuedConnection);
}

void AalImageCaptureControl::saveJpegCB(void *data, uint32_t data_size, void *context)
{
    Q_UNUSED(context);
    AalCameraService::instance()->imageCaptureControl()->saveJpeg(data, data_size);
}

void AalImageCaptureControl::init(CameraControl *control, CameraControlListener *listener)
{
    if (m_service->deviceSelector()->selectedDevice() == 0) {
        m_photoWidth = 2592;
        m_photoHeight = 1458;
    } else {
        m_photoWidth = 1280;
        m_photoHeight = 960;
    }
    android_camera_set_picture_size(control, m_photoWidth, m_photoHeight);

    listener->on_msg_shutter_cb = &AalImageCaptureControl::shutterCB;
    listener->on_data_compressed_image_cb = &AalImageCaptureControl::saveJpegCB;
}

void AalImageCaptureControl::updateReady()
{
    bool ready = calculateReadyStatus();
    if (m_ready != ready) {
        m_ready = ready;
        Q_EMIT readyForCaptureChanged(m_ready);
    }
}

bool AalImageCaptureControl::calculateReadyStatus()
{
    if (!m_cameraControl->state() == QCamera::ActiveState)
        return false;
    if (!m_pendingCaptureFile.isNull())
        return false;
    if (m_service->focusControl()->isFocusBusy())
        return false;
    if (!m_service->videoOutputControl()->isViewfinderRunning())
        return false;

    return true;
}

void AalImageCaptureControl::shutter()
{
    Q_EMIT imageExposed(m_lastRequestId);
    m_service->videoOutputControl()->createPreview();
    Q_EMIT imageCaptured(m_lastRequestId, m_service->videoOutputControl()->preview());
}

void AalImageCaptureControl::saveJpeg(void *data, uint32_t data_size)
{
    if (m_pendingCaptureFile.isNull() || !m_service->androidControl())
        return;

    QTemporaryFile file;
    if (!file.open()) {
        qWarning() << "Could not save image to " << m_pendingCaptureFile;
        m_pendingCaptureFile.clear();
        updateReady();
        return;
    }

    file.write((const char*)data, data_size);
    file.close();

    QFile finalFile(file.fileName());
    finalFile.rename(m_pendingCaptureFile);

    Q_EMIT imageSaved(m_lastRequestId, m_pendingCaptureFile);
    m_pendingCaptureFile.clear();

    android_camera_start_preview(m_service->androidControl());
    updateReady();
}
