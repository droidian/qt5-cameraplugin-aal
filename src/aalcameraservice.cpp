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
#include "aalcameraservice.h"
#include "aalimagecapturecontrol.h"
#include "aalvideorenderercontrol.h"

#include "camera_compatibility_layer.h"

#include <QDebug>


void error_msg_cb(void* context)
{
    printf("%s \n", __PRETTY_FUNCTION__);
}

void zoom_msg_cb(void* context, int32_t new_zoom_level)
{
    printf("%s \n", __PRETTY_FUNCTION__);
}

void autofocus_msg_cb(void* context)
{
    printf("%s \n", __PRETTY_FUNCTION__);
}

AalCameraService *AalCameraService::m_service = 0;

AalCameraService::AalCameraService(QObject *parent):
    QMediaService(parent)
{
    m_service = this;

    m_cameraListener = new CameraControlListener;
    memset(m_cameraListener, 0, sizeof(*m_cameraListener));

    m_cameraControl = new AalCameraControl(this);
    m_imageCaptureControl = new AalImageCaptureControl(this);
    m_videoOutput = new AalVideoRendererControl(this);

    m_cameraListener->on_msg_error_cb = error_msg_cb;
    m_cameraListener->on_msg_focus_cb = autofocus_msg_cb;
    m_cameraListener->on_msg_zoom_cb = zoom_msg_cb;
}

AalCameraService::~AalCameraService()
{
    m_cameraControl->setState(QCamera::UnloadedState);
    delete m_cameraControl;
    delete m_imageCaptureControl;
    delete m_videoOutput;
}

QMediaControl *AalCameraService::requestControl(const char *name)
{
    if (qstrcmp(name, QCameraControl_iid) == 0)
        return m_cameraControl;

    if (qstrcmp(name, QCameraImageCaptureControl_iid) == 0)
        return m_imageCaptureControl;

    if (qstrcmp(name, QVideoRendererControl_iid) == 0)
        return m_videoOutput;

    return 0;
}

void AalCameraService::releaseControl(QMediaControl *control)
{
    Q_UNUSED(control);
}

