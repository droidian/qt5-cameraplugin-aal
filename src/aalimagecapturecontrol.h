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

#ifndef AALIMAGECAPTURECONTROL_H
#define AALIMAGECAPTURECONTROL_H

#include <QCameraImageCaptureControl>
#include <QString>
#include <storagemanager.h>

#include <stdint.h>

class AalCameraService;
class AalCameraControl;
class AalImageEncoderControl;
class CameraControl;
class CameraControlListener;

class AalImageCaptureControl : public QCameraImageCaptureControl
{
Q_OBJECT
public:
    AalImageCaptureControl(AalCameraService *service, QObject *parent = 0);
    ~AalImageCaptureControl();

    int capture(const QString &fileName);
    void cancelCapture();

    bool isReadyForCapture() const;

    QCameraImageCapture::DriveMode driveMode() const { return QCameraImageCapture::SingleImageCapture; }
    void setDriveMode(QCameraImageCapture::DriveMode ) {}

    static void shutterCB(void* context);
    static void saveJpegCB(void* data, uint32_t data_size, void* context);

    void setReady(bool ready);

    bool isCaptureRunning() const;

public Q_SLOTS:
    void init(CameraControl *control, CameraControlListener *listener);

private Q_SLOTS:
    void shutter();

private:
    QSize chooseOptimalSize(const QList<QSize> &sizes);
    void saveJpeg(void* data, uint32_t dataSize);
    bool imageIsInGallery(const QString &fileName) const;
    bool saveThumbnail(const uchar *data, int dataSize);

    AalCameraService *m_service;
    AalCameraControl *m_cameraControl;
    AalImageEncoderControl *m_imageEncoderControl;
    int m_lastRequestId;
    StorageManager m_storageManager;
    bool m_ready;
    QString m_pendingCaptureFile;
    int m_photoWidth;
    int m_photoHeight;
    QString m_galleryPath;
};

#endif
