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
#include "aalvideorenderercontrol.h"

#include "camera_compatibility_layer.h"

#include <QDebug>


void error_msg_cb(void* context)
{
    printf("%s \n", __PRETTY_FUNCTION__);
}

AalCameraService *AalCameraService::m_service = 0;

AalCameraService::AalCameraService(QObject *parent):
    QMediaService(parent),
    m_androidControl(0)
{
    m_service = this;

    m_cameraListener = new CameraControlListener;
    memset(m_cameraListener, 0, sizeof(*m_cameraListener));

    m_cameraControl = new AalCameraControl(this);
    m_flashControl = new AalCameraFlashControl(this);
    m_focusControl = new AalCameraFocusControl(this);
    m_zoomControl = new AalCameraZoomControl(this);
    m_imageCaptureControl = new AalImageCaptureControl(this);
    m_videoOutput = new AalVideoRendererControl(this);

    m_cameraListener->on_msg_error_cb = error_msg_cb;
}

AalCameraService::~AalCameraService()
{
    m_cameraControl->setState(QCamera::UnloadedState);
    delete m_cameraControl;
    delete m_flashControl;
    delete m_focusControl;
    delete m_zoomControl;
    delete m_imageCaptureControl;
    delete m_videoOutput;
    delete m_androidControl;
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

    m_androidControl = android_camera_connect_to(FRONT_FACING_CAMERA_TYPE, m_cameraListener);
    if (!m_androidControl) {
        qWarning() << "Unable to connect to camera";
        return false;
    }

    m_cameraListener->context = m_androidControl;
    m_flashControl->init(m_androidControl);
    m_focusControl->init(m_androidControl, m_cameraListener);
    m_zoomControl->init(m_androidControl, m_cameraListener);
    m_videoOutput->startPreview();

    return true;
}
