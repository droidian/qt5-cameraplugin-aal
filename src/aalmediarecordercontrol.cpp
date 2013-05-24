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

#include "aalcameraservice.h"
#include "aalmediarecordercontrol.h"
#include "storagemanager.h"

#include <QDebug>
#include <QFile>
#include <QTimer>

#include <camera_compatibility_layer.h>
#include <recorder_compatibility_layer.h>

#include <fcntl.h>

/*!
 * \brief AalMediaRecorderControl::AalMediaRecorderControl
 * \param service
 * \param parent
 */
AalMediaRecorderControl::AalMediaRecorderControl(AalCameraService *service, QObject *parent)
   : QMediaRecorderControl(parent),
    m_service(service),
    m_mediaRecorder(0),
    m_duration(0),
    m_currentState(QMediaRecorder::StoppedState),
    m_currentStatus(QMediaRecorder::UnloadedStatus),
    m_recordingTimer(0)
{
}

/*!
 * \brief AalMediaRecorderControl::~AalMediaRecorderControl
 */
AalMediaRecorderControl::~AalMediaRecorderControl()
{
    delete m_recordingTimer;
    deleteRecorder();
}

/*!
 * \reimp
 */
void AalMediaRecorderControl::applySettings()
{
    qDebug() << Q_FUNC_INFO << " is not used";
}

/*!
 * \reimp
 */
qint64 AalMediaRecorderControl::duration() const
{
    return m_duration;
}

/*!
 * \reimp
 */
bool AalMediaRecorderControl::isMuted() const
{
    qDebug() << Q_FUNC_INFO << " is not used";
    return false;
}

/*!
 * \reimp
 */
QUrl AalMediaRecorderControl::outputLocation() const
{
    return m_outputLocation;
}

/*!
 * \reimp
 */
bool AalMediaRecorderControl::setOutputLocation(const QUrl &location)
{
    if ( m_outputLocation == location)
        return true;

    m_outputLocation = location;
    Q_EMIT actualLocationChanged(m_outputLocation);
    return true;
}

/*!
 * \reimp
 */
QMediaRecorder::State AalMediaRecorderControl::state() const
{
    return m_currentState;
}

/*!
 * \reimp
 */
QMediaRecorder::Status AalMediaRecorderControl::status() const
{
    return m_currentStatus;
}

/*!
 * \reimp
 */
qreal AalMediaRecorderControl::volume() const
{
    qDebug() << Q_FUNC_INFO << " is not used";
    return 1.0;
}

/*!
 * \brief AalMediaRecorderControl::init makes sure the mediearecoder is
 * initialized
 */
void AalMediaRecorderControl::init()
{
    if (m_mediaRecorder == 0) {
        m_mediaRecorder = android_media_new_recorder();

        if (m_mediaRecorder == 0) {
            qWarning() << "Unanble to create new media recorder";
            Q_EMIT error(-1, "Unanble to create new media recorder");
        } else {
            setStatus(QMediaRecorder::LoadedStatus);
            android_recorder_set_error_cb(m_mediaRecorder, &AalMediaRecorderControl::errorCB, this);
        }
    }

    if (m_recordingTimer == 0) {
        m_recordingTimer = new QTimer(this);
        m_recordingTimer->setInterval(1000);
        m_recordingTimer->setSingleShot(false);
        QObject::connect(m_recordingTimer, SIGNAL(timeout()),
                         this, SLOT(updateDuration()));
    }
}

/*!
 * \brief AalMediaRecorderControl::deleteRecorder releases all resources and
 * deletes the MediaRecorder
 */
void AalMediaRecorderControl::deleteRecorder()
{
    if (m_mediaRecorder == 0)
        return;

    android_recorder_release(m_mediaRecorder);
    m_mediaRecorder = 0;
}

/*!
 * \brief AalMediaRecorderControl::errorCB handles errors from the android layer
 * \param context
 */
void AalMediaRecorderControl::errorCB(void *context)
{
    Q_UNUSED(context);
    QMetaObject::invokeMethod(AalCameraService::instance()->mediaRecorderControl(),
                              "handleError", Qt::QueuedConnection);
}

/*!
 * \reimp
 */
void AalMediaRecorderControl::setMuted(bool muted)
{
    Q_UNUSED(muted);
    qDebug() << Q_FUNC_INFO << " is not used";
}

/*!
 * \reimp
 */
void AalMediaRecorderControl::setState(QMediaRecorder::State state)
{
    if (m_currentState == state)
        return;

    switch (state) {
    case QMediaRecorder::RecordingState: {
        int ret = startRecording();
        if (ret == -1) {
            setStatus(QMediaRecorder::LoadedStatus);
        }
        break;
    }
    case QMediaRecorder::StoppedState: {
        stopRecording();
        break;
    }
    case QMediaRecorder::PausedState: {
        qDebug() << Q_FUNC_INFO << " pause not used for video recording.";
        break;
    }
    }
}

/*!
 * \reimp
 */
void AalMediaRecorderControl::setVolume(qreal gain)
{
    Q_UNUSED(gain);
    qDebug() << Q_FUNC_INFO << " is not used";
}

void AalMediaRecorderControl::updateDuration()
{
    m_duration += 1000;
    Q_EMIT durationChanged(m_duration);
}

/*!
 * \brief AalMediaRecorderControl::handleError emits errors from android layer
 */
void AalMediaRecorderControl::handleError()
{
    Q_EMIT error(-1, "Error on recording video");
}

/*!
 * \brief AalMediaRecorderControl::setStatus
 * \param status
 */
void AalMediaRecorderControl::setStatus(QMediaRecorder::Status status)
{
    if (m_currentStatus == status)
        return;

    m_currentStatus = status;
    Q_EMIT statusChanged(m_currentStatus);
}

/*!
 * \brief AalMediaRecorderControl::startRecording
 */
int AalMediaRecorderControl::startRecording()
{    
    if (m_service->androidControl() == 0) {
        Q_EMIT error(-1, "No camera connection");
        return -2;
    }

    init();
    if (m_mediaRecorder == 0) {
        setStatus(QMediaRecorder::UnloadedStatus);
        return -2;
    }

    setStatus(QMediaRecorder::StartingStatus);

    m_duration = 0;

    android_camera_unlock(m_service->androidControl());

    int ret;
    ret = android_recorder_setCamera(m_mediaRecorder, m_service->androidControl());
    if (ret < 0) {
        Q_EMIT error(-1, "android_recorder_setCamera() failed\n");
        return -1;
    }
    //state initial / idle
    ret = android_recorder_setAudioSource(m_mediaRecorder, ANDROID_AUDIO_SOURCE_CAMCORDER);
    if (ret < 0) {
        Q_EMIT error(-1, "android_recorder_setAudioSource() failed");
        return -1;
    }
    ret = android_recorder_setVideoSource(m_mediaRecorder, ANDROID_VIDEO_SOURCE_CAMERA);
    if (ret < 0) {
        Q_EMIT error(-1, "android_recorder_setVideoSource() failed");
        return -1;
    }
    //state initialized
    ret = android_recorder_setOutputFormat(m_mediaRecorder, ANDROID_OUTPUT_FORMAT_MPEG_4);
    if (ret < 0) {
        Q_EMIT error(-1, "android_recorder_setOutputFormat() failed");
        return -1;
    }
    //state DataSourceConfigured
    ret = android_recorder_setAudioEncoder(m_mediaRecorder, ANDROID_AUDIO_ENCODER_AAC);
    if (ret < 0) {
        Q_EMIT error(-1, "android_recorder_setAudioEncoder() failed");
        return -1;
    }
    ret = android_recorder_setVideoEncoder(m_mediaRecorder, ANDROID_VIDEO_ENCODER_H264);
    if (ret < 0) {
        Q_EMIT error(-1, "android_recorder_setVideoEncoder() failed");
        return -1;
    }

    QString fileName = m_outputLocation.path();
    if (fileName.isEmpty() || QFile::exists(fileName)) {
        fileName = m_service->storageManager()->nextVideoFileName();
    }
    int fd;
    fd = open(fileName.toLocal8Bit().data(), O_WRONLY | O_CREAT);
    if (fd < 0) {
        Q_EMIT error(-1, "Could not open file for video recording");
        return -1;
    }
    QFile::setPermissions(fileName, QFile::ReadOwner | QFile::WriteOwner |
                               QFile::ReadGroup | QFile::ReadOther);
    ret = android_recorder_setOutputFile(m_mediaRecorder, fd);
    if (ret < 0) {
        Q_EMIT error(-1, "android_recorder_setOutputFile() failed");
        return -1;
    }

    ret = android_recorder_setVideoSize(m_mediaRecorder, 1280, 720);
    if (ret < 0) {
        Q_EMIT error(-1, "android_recorder_setVideoSize() failed");
        return -1;
    }
    ret = android_recorder_setVideoFrameRate(m_mediaRecorder, 30);
    if (ret < 0) {
        Q_EMIT error(-1, "android_recorder_setVideoFrameRate() failed");
        return -1;
    }

    ret = android_recorder_prepare(m_mediaRecorder);
    if (ret < 0) {
        Q_EMIT error(-1, "android_recorder_prepare() failed");
        return -1;
    }
    //state prepared
    ret = android_recorder_start(m_mediaRecorder);
    if (ret < 0) {
        Q_EMIT error(-1, "android_recorder_start() failed");
        return -1;
    }

    m_currentState = QMediaRecorder::RecordingState;
    Q_EMIT stateChanged(m_currentState);

    setStatus(QMediaRecorder::RecordingStatus);

    m_recordingTimer->start();

    return 0;
}

/*!
 * \brief AalMediaRecorderControl::stopRecording
 */
void AalMediaRecorderControl::stopRecording()
{
    if (m_mediaRecorder == 0) {
        return;
    }

    setStatus(QMediaRecorder::FinalizingStatus);
    m_recordingTimer->stop();

    int result = android_recorder_stop(m_mediaRecorder);
    if (result < 0) {
        Q_EMIT error(-1, "Cannot stop video recording");
        return;
    }

    android_recorder_reset(m_mediaRecorder);
    android_camera_lock(m_service->androidControl());

    m_currentState = QMediaRecorder::StoppedState;
    Q_EMIT stateChanged(m_currentState);
    setStatus(QMediaRecorder::LoadedStatus);
}
