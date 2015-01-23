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

#include "aalmediarecordercontrol.h"
#include "aalcameraservice.h"
#include "aalmetadatawritercontrol.h"
#include "aalvideoencodersettingscontrol.h"
#include "aalviewfindersettingscontrol.h"
#include "audiocapture.h"
#include "storagemanager.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QTimer>

#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/camera/camera_compatibility_layer_capabilities.h>
#include <hybris/media/media_recorder_layer.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

const int AalMediaRecorderControl::RECORDER_GENERAL_ERROR;
const int AalMediaRecorderControl::RECORDER_NOT_AVAILABLE_ERROR;
const int AalMediaRecorderControl::RECORDER_INITIALIZATION_ERROR;

const int AalMediaRecorderControl::DURATION_UPDATE_INTERVAL;

const QLatin1String AalMediaRecorderControl::PARAM_AUDIO_BITRATE = QLatin1String("audio-param-encoding-bitrate");
const QLatin1String AalMediaRecorderControl::PARAM_AUDIO_CHANNELS = QLatin1String("audio-param-number-of-channels");
const QLatin1String AalMediaRecorderControl::PARAM_AUTIO_SAMPLING = QLatin1String("audio-param-sampling-rate");
const QLatin1String AalMediaRecorderControl::PARAM_LATITUDE = QLatin1String("param-geotag-latitude");
const QLatin1String AalMediaRecorderControl::PARAM_LONGITUDE = QLatin1String("param-geotag-longitude");
const QLatin1String AalMediaRecorderControl::PARAM_ORIENTATION = QLatin1String("video-param-rotation-angle-degrees");
const QLatin1String AalMediaRecorderControl::PARAM_VIDEO_BITRATE = QLatin1String("video-param-encoding-bitrate");
/*!
 * \brief AalMediaRecorderControl::AalMediaRecorderControl
 * \param service
 * \param parent
 */
AalMediaRecorderControl::AalMediaRecorderControl(AalCameraService *service, QObject *parent)
   : QMediaRecorderControl(parent),
    m_service(service),
    m_mediaRecorder(0),
    m_audioCapture(0),
    m_outfd(-1),
    m_duration(0),
    m_currentState(QMediaRecorder::StoppedState),
    m_currentStatus(QMediaRecorder::UnloadedStatus),
    m_recordingTimer(0),
    m_workerThread(0)
{
}

/*!
 * \brief AalMediaRecorderControl::~AalMediaRecorderControl
 */
AalMediaRecorderControl::~AalMediaRecorderControl()
{
    delete m_recordingTimer;
    if (m_outfd != -1)
    {
        int err = close(m_outfd);
        if (err < 0)
            qWarning() << "Failed to close recording output file descriptor (errno: "
                << errno << ")";
    }
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
 * \brief Starts the main microphone reader/writer loop in AudioCapture (run)
 */
void AalMediaRecorderControl::onStartThread()
{
    qDebug() << "Starting microphone reader/writer worker thread";
    // Start the microphone read/write worker thread
    m_workerThread->start();
    Q_EMIT startWorkerThread();
}

/*!
 * \brief AalMediaRecorderControl::init makes sure the mediarecorder is
 * initialized
 */
void AalMediaRecorderControl::initRecorder()
{
    if (m_mediaRecorder == 0) {
        m_mediaRecorder = android_media_new_recorder();

        m_audioCapture = new AudioCapture(m_mediaRecorder);
        m_workerThread = new QThread;

        if (m_audioCapture == 0) {
            qWarning() << "Unable to create new audio capture, audio recording won't function";
            Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "Unable to create new audio capture, audio recording won't function");
        }
        else
        {
            bool ret = false;

            // Make sure that m_audioCapture is executed within the m_workerThread affinity
            m_audioCapture->moveToThread(m_workerThread);

            // Finished signal is for when the workerThread is completed. Important to connect this so that
            // resources are cleaned up in the proper order and not leaked
            ret = connect(m_audioCapture, SIGNAL(finished()), m_workerThread, SLOT(quit()));
            if (!ret)
                qWarning() << "Failed to connect quit() to the m_audioCapture finished signal";
            ret = connect(m_audioCapture, SIGNAL(finished()), m_audioCapture, SLOT(deleteLater()));
            if (!ret)
                qWarning() << "Failed to connect deleteLater() to the m_audioCapture finished signal";
            // Clean up the worker thread after we finish recording
            ret = connect(m_workerThread, SIGNAL(finished()), m_workerThread, SLOT(deleteLater()));
            if (!ret)
                qWarning() << "Failed to connect deleteLater() to the m_workerThread finished signal";
            // startWorkerThread signal comes from an Android layer callback that resides down in
            // the AudioRecordHybris class
            ret = connect(this, SIGNAL(startWorkerThread()), m_audioCapture, SLOT(run()));
            if (!ret)
                qWarning() << "Failed to connect run() to the local startWorkerThread signal";

            // Call onStartThreadCb when the reader side of the named pipe has been setup
            m_audioCapture->init(&AalMediaRecorderControl::onStartThreadCb, this);
        }

        if (m_mediaRecorder == 0) {
            qWarning() << "Unable to create new media recorder";
            Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "Unable to create new media recorder");
        } else {
            setStatus(QMediaRecorder::LoadedStatus);
            android_recorder_set_error_cb(m_mediaRecorder, &AalMediaRecorderControl::errorCB, this);
            android_camera_unlock(m_service->androidControl());
        }
    }

    if (m_recordingTimer == 0) {
        m_recordingTimer = new QTimer(this);
        m_recordingTimer->setInterval(DURATION_UPDATE_INTERVAL);
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
    android_camera_lock(m_service->androidControl());
    setStatus(QMediaRecorder::UnloadedStatus);
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

MediaRecorderWrapper* AalMediaRecorderControl::mediaRecorder() const
{
    return m_mediaRecorder;
}

AudioCapture *AalMediaRecorderControl::audioCapture() const
{
    return m_audioCapture;
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
    m_duration += DURATION_UPDATE_INTERVAL;
    Q_EMIT durationChanged(m_duration);
}

/*!
 * \brief AalMediaRecorderControl::handleError emits errors from android layer
 */
void AalMediaRecorderControl::handleError()
{
    Q_EMIT error(RECORDER_GENERAL_ERROR, "Error on recording video");
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
 * \brief AalMediaRecorderControl::startRecording starts a video record.
 * FIXME add support for recording audio only
 */
int AalMediaRecorderControl::startRecording()
{
    if (m_service->androidControl() == 0) {
        Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "No camera connection");
        return RECORDER_INITIALIZATION_ERROR;
    }

    m_duration = 0;
    Q_EMIT durationChanged(m_duration);

    initRecorder();
    if (m_mediaRecorder == 0) {
        deleteRecorder();
        return RECORDER_NOT_AVAILABLE_ERROR;
    }

    setStatus(QMediaRecorder::StartingStatus);

    QVideoEncoderSettings videoSettings = m_service->videoEncoderControl()->videoSettings();

    int ret;
    ret = android_recorder_setCamera(m_mediaRecorder, m_service->androidControl());
    if (ret < 0) {
        deleteRecorder();
        Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "android_recorder_setCamera() failed\n");
        return RECORDER_INITIALIZATION_ERROR;
    }
    //state initial / idle
    ret = android_recorder_setAudioSource(m_mediaRecorder, ANDROID_AUDIO_SOURCE_CAMCORDER);
    if (ret < 0) {
        deleteRecorder();
        Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "android_recorder_setAudioSource() failed");
        return RECORDER_INITIALIZATION_ERROR;
    }
    ret = android_recorder_setVideoSource(m_mediaRecorder, ANDROID_VIDEO_SOURCE_CAMERA);
    if (ret < 0) {
        deleteRecorder();
        Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "android_recorder_setVideoSource() failed");
        return RECORDER_INITIALIZATION_ERROR;
    }
    //state initialized
    ret = android_recorder_setOutputFormat(m_mediaRecorder, ANDROID_OUTPUT_FORMAT_MPEG_4);
    if (ret < 0) {
        deleteRecorder();
        Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "android_recorder_setOutputFormat() failed");
        return RECORDER_INITIALIZATION_ERROR;
    }
    //state DataSourceConfigured
    ret = android_recorder_setAudioEncoder(m_mediaRecorder, ANDROID_AUDIO_ENCODER_AAC);
    if (ret < 0) {
        deleteRecorder();
        Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "android_recorder_setAudioEncoder() failed");
        return RECORDER_INITIALIZATION_ERROR;
    }
    // FIXME set codec from settings
    ret = android_recorder_setVideoEncoder(m_mediaRecorder, ANDROID_VIDEO_ENCODER_H264);
    if (ret < 0) {
        deleteRecorder();
        Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "android_recorder_setVideoEncoder() failed");
        return RECORDER_INITIALIZATION_ERROR;
    }

    QString fileName = m_outputLocation.path();
    QFileInfo fileInfo = QFileInfo(fileName);
    if (fileName.isEmpty()) {
        fileName = m_service->storageManager()->nextVideoFileName();
    } else if (fileInfo.isDir()) {
        fileName = m_service->storageManager()->nextVideoFileName(fileName);
    }

    m_outfd = open(fileName.toLocal8Bit().data(), O_WRONLY | O_CREAT,
              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (m_outfd < 0) {
        deleteRecorder();
        Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "Could not open file for video recording");
        return RECORDER_INITIALIZATION_ERROR;
    }
    ret = android_recorder_setOutputFile(m_mediaRecorder, m_outfd);
    if (ret < 0) {
        close(m_outfd);
        deleteRecorder();
        Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "android_recorder_setOutputFile() failed");
        return RECORDER_INITIALIZATION_ERROR;
    }

    QSize resolution = videoSettings.resolution();
    ret = android_recorder_setVideoSize(m_mediaRecorder, resolution.width(), resolution.height());
    if (ret < 0) {
        close(m_outfd);
        deleteRecorder();
        Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "android_recorder_setVideoSize() failed");
        return RECORDER_INITIALIZATION_ERROR;
    }
    ret = android_recorder_setVideoFrameRate(m_mediaRecorder, videoSettings.frameRate());
    if (ret < 0) {
        close(m_outfd);
        deleteRecorder();
        Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "android_recorder_setVideoFrameRate() failed");
        return RECORDER_INITIALIZATION_ERROR;
    }

    setParameter(PARAM_VIDEO_BITRATE, videoSettings.bitRate());
    // FIXME get data from a new AalAudioEncoderSettingsControl
    setParameter(PARAM_AUDIO_BITRATE, 48000);
    setParameter(PARAM_AUDIO_CHANNELS, 2);
    setParameter(PARAM_AUTIO_SAMPLING, 96000);
    if (m_service->metadataWriterControl()) {
        int rotation = m_service->metadataWriterControl()->correctedOrientation();
        setParameter(PARAM_ORIENTATION, rotation);
        m_service->metadataWriterControl()->clearAllMetaData();
    }

    ret = android_recorder_prepare(m_mediaRecorder);
    if (ret < 0) {
        close(m_outfd);
        deleteRecorder();
        Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "android_recorder_prepare() failed");
        return RECORDER_INITIALIZATION_ERROR;
    }
    //state prepared
    ret = android_recorder_start(m_mediaRecorder);
    if (ret < 0) {
        close(m_outfd);
        deleteRecorder();
        Q_EMIT error(RECORDER_INITIALIZATION_ERROR, "android_recorder_start() failed");
        return RECORDER_INITIALIZATION_ERROR;
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
    qDebug() << __PRETTY_FUNCTION__;
    if (m_mediaRecorder == 0) {
        qWarning() << "Can't stop recording properly, m_mediaRecorder is NULL";
        return;
    }
    if (m_audioCapture == 0) {
        qWarning() << "Can't stop recording properly, m_audioCapture is NULL";
        return;
    }

    setStatus(QMediaRecorder::FinalizingStatus);
    m_recordingTimer->stop();

    int result = android_recorder_stop(m_mediaRecorder);
    if (result < 0) {
        Q_EMIT error(RECORDER_GENERAL_ERROR, "Cannot stop video recording");
        return;
    }

    // Stop microphone reader/writer loop
    // NOTE: This must come after the android_recorder_stop call, otherwise the
    // RecordThread instance will block the MPEG4Writer pthread_join when trying to
    // cleanly stop recording.
    m_audioCapture->stopCapture();

    android_recorder_reset(m_mediaRecorder);

    int err = close(m_outfd);
    if (err < 0)
        qWarning() << "Failed to close recording output file descriptor (errno: "
            << errno << ")";
    m_outfd = -1;

    m_currentState = QMediaRecorder::StoppedState;
    Q_EMIT stateChanged(m_currentState);

    deleteRecorder();
}

/*!
 * \brief AalMediaRecorderControl::setParameter convenient function to set parameters
 * \param parameter Name of the parameter
 * \param value value to set
 */
void AalMediaRecorderControl::setParameter(const QString &parameter, int value)
{
    Q_ASSERT(m_mediaRecorder);
    QString param =  parameter + QChar('=') + QString::number(value);
    android_recorder_setParameters(m_mediaRecorder, param.toLocal8Bit().data());
}

void AalMediaRecorderControl::onStartThreadCb(void *context)
{
    AalMediaRecorderControl *thiz = static_cast<AalMediaRecorderControl*>(context);
    if (thiz != NULL)
        thiz->onStartThread();
}
