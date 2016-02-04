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

#ifndef AALIMAGECAPTURECONTROL_H
#define AALIMAGECAPTURECONTROL_H

#include <QCameraImageCaptureControl>
#include <QSettings>
#include <QString>
#include <QFutureWatcher>
#include <storagemanager.h>

#include <stdint.h>

class AalCameraService;
class AalCameraControl;
class CameraControl;
class CameraControlListener;
class QMediaPlayer;

typedef QFutureWatcher<QString> StringFutureWatcher;

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
    void onPreviewReady();
    void onImageFileSaved();

private Q_SLOTS:
    void shutter();
    void saveJpeg(const QByteArray& data);

private:
    bool updateJpegMetadata(void* data, uint32_t dataSize, QTemporaryFile* destination);

    AalCameraService *m_service;
    AalCameraControl *m_cameraControl;
    int m_lastRequestId;
    StorageManager m_storageManager;
    bool m_ready;
    QString m_targetFileName;
    QString m_pendingCaptureFile;
    bool m_captureCancelled;
    float m_screenAspectRatio;
    /// Maintains a list of highest priority aspect ratio to lowest, for the
    /// currently selected camera
    QList<float> m_prioritizedAspectRatios;
    QString m_galleryPath;
    QMediaPlayer *m_audioPlayer;
    QSettings m_settings;

    QMap<StringFutureWatcher*, int> m_pendingSaveOperations;
};

#endif
