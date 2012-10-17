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

#ifndef AALCAMERASERVICE_H
#define AALCAMERASERVICE_H

#include <QMediaService>

class AalCameraControl;
class AalImageCaptureControl;
class AalVideoRendererControl;
class QCameraControl;

struct CameraControlListener;

class AalCameraService : public QMediaService
{
Q_OBJECT
public:
    AalCameraService(QObject *parent = 0);
    ~AalCameraService();

    QMediaControl* requestControl(const char *name);
    void releaseControl(QMediaControl *control);

    AalCameraControl *cameraControl() const { return m_cameraControl; }
    AalImageCaptureControl *imageCaptureControl() const { return m_imageCaptureControl; }
    AalVideoRendererControl *videoOutputControl() const { return m_videoOutput; }

    CameraControlListener *listener() {return m_cameraListener; }

    static AalCameraService *instance() { return m_service; }

private:
    static AalCameraService *m_service;

    AalCameraControl *m_cameraControl;
    AalImageCaptureControl *m_imageCaptureControl;
    AalVideoRendererControl *m_videoOutput;

    CameraControlListener *m_cameraListener;
};

#endif
