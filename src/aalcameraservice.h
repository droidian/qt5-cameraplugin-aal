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

#ifndef AALCAMERASERVICE_H
#define AALCAMERASERVICE_H

#include <QMediaService>

class AalCameraControl;
class AalCameraFlashControl;
class AalCameraFocusControl;
class AalCameraZoomControl;
class AalImageCaptureControl;
class AalImageEncoderControl;
class AalMediaRecorderControl;
class AalMetaDataWriterControl;
class AalVideoDeviceSelectorControl;
class AalVideoRendererControl;
class AalViewfinderSettingsControl;
class QCameraControl;

struct CameraControl;
struct CameraControlListener;

class StorageManager;

class AalCameraService : public QMediaService
{
Q_OBJECT
public:
    AalCameraService(QObject *parent = 0);
    ~AalCameraService();

    QMediaControl* requestControl(const char *name);
    void releaseControl(QMediaControl *control);

    AalCameraControl *cameraControl() const { return m_cameraControl; }
    AalCameraFlashControl *flashControl() const { return m_flashControl; }
    AalCameraFocusControl *focusControl() const { return m_focusControl; }
    AalCameraZoomControl *zoomControl() const { return m_zoomControl; }
    AalImageCaptureControl *imageCaptureControl() const { return m_imageCaptureControl; }
    AalImageEncoderControl *imageEncoderControl() const { return m_imageEncoderControl; }
    AalMediaRecorderControl *mediaRecorderControl() const { return m_mediaRecorderControl; }
    AalMetaDataWriterControl *metadataWriterControl() const { return m_metadataWriter; }
    AalVideoDeviceSelectorControl *deviceSelector() const { return m_deviceSelectControl; }
    AalVideoRendererControl *videoOutputControl() const { return m_videoOutput; }
    AalViewfinderSettingsControl *viewfinderControl() const { return m_viewfinderControl; }

    CameraControl *androidControl();

    StorageManager *storageManager();

    bool connectCamera();
    void disconnectCamera();

    bool isCameraActive() const;
    bool isBackCameraUsed() const;

    static AalCameraService *instance() { return m_service; }

public Q_SLOTS:
    void updateCaptureReady();

private:
    void initControls(CameraControl *camControl, CameraControlListener *listener);

    static AalCameraService *m_service;

    AalCameraControl *m_cameraControl;
    AalCameraFlashControl *m_flashControl;
    AalCameraFocusControl *m_focusControl;
    AalCameraZoomControl *m_zoomControl;
    AalImageCaptureControl *m_imageCaptureControl;
    AalImageEncoderControl *m_imageEncoderControl;
    AalMediaRecorderControl *m_mediaRecorderControl;
    AalMetaDataWriterControl *m_metadataWriter;
    AalVideoDeviceSelectorControl *m_deviceSelectControl;
    AalVideoRendererControl *m_videoOutput;
    AalViewfinderSettingsControl *m_viewfinderControl;

    CameraControl *m_androidControl;
    CameraControlListener *m_androidListener;
    CameraControl *m_oldAndroidControl;

    StorageManager *m_storageManager;
};

#endif
