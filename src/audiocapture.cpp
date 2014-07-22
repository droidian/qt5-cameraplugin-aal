/*
 * Copyright (C) 2014 Canonical, Ltd.
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

#include "audiocapture.h"

#include <pulse/simple.h>
#include <pulse/error.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <QDebug>
#include <QThread>

AudioCapture::AudioCapture(MediaRecorderWrapper *mediaRecorder)
    : m_paStream(NULL),
      m_mediaRecorder(mediaRecorder),
      m_audioPipe(-1),
      m_startWorkThreadCb(NULL),
      m_startWorkThreadContext(NULL)
{
    qDebug() << "Instantiating new AudioCapture instance";
}

AudioCapture::~AudioCapture()
{
    if (m_audioPipe >= 0)
        close(m_audioPipe);
    //delete m_audioBuf;
    if (m_paStream != NULL)
        pa_simple_free(m_paStream);
}

bool AudioCapture::init()
{
    // The MediaRecorderLayer will call method (onMicrophoneRead) when it's ready to encode a new audio buffer
    android_recorder_set_audio_read_cb(m_mediaRecorder, &AudioCapture::onReadMicrophone, this);

    if (!setupMicrophoneStream())
    {
        qWarning() << "Failed to setup PulseAudio microphone recording stream";
        return false;
    }

    return true;
}

void AudioCapture::run()
{
    qDebug() << __PRETTY_FUNCTION__;

    int bytesWritten = 0, bytesRead = 0;

    if (!setupPipe())
    {
        qWarning() << "Failed to open /dev/socket/micshm, cannot write data to pipe";
        goto exit;
    }

//    do {
        qDebug() << "--> reading from the mic";
        bytesRead = readMicrophone();
        if (bytesRead > 0)
        {
            qDebug() << "--> writing to the pipe";
            bytesWritten = writeDataToPipe();
        }
//    } while (bytesRead == MIC_READ_BUF_SIZE && bytesWritten == MIC_READ_BUF_SIZE);

    qWarning() << "Broke out of the AudioCapture thread loop, signaling finish";

exit:
    Q_EMIT finished();
}

void AudioCapture::moveToThread(QThread *thread)
{
    moveToThread(thread);
}

void AudioCapture::setStartWorkerThreadCb(StartWorkerThreadCb cb, void *context)
{
    qDebug() << __PRETTY_FUNCTION__;
    m_startWorkThreadCb = cb;
    m_startWorkThreadContext = context;
}

int AudioCapture::readMicrophone()
{
    qDebug() << __PRETTY_FUNCTION__;

    int ret = 0, error = 0;
    // Reinitialize the audio buffer
    std::fill_n(m_audioBuf, sizeof(m_audioBuf), 0);
    qDebug() << "Reading microphone data...";
    ret = pa_simple_read(m_paStream, m_audioBuf, sizeof(m_audioBuf), &error);
    if (ret < 0)
    {
        //qWarning() << "Failed to read audio from the microphone: " << pa_strerror(error);
        qWarning() << "Failed to read audio from the microphone: ";
        goto exit;
    }
    else
        ret = sizeof(m_audioBuf);

    qDebug() << "Read in " << ret << " bytes";

exit:
    return ret;
}

void AudioCapture::startThreadLoop()
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_EMIT startThread();
    if (m_startWorkThreadCb != NULL)
    {
        m_startWorkThreadCb(m_startWorkThreadContext);
    }
    else
        qWarning() << "Couldn't start worker thread since m_startWorkThreadCb is NULL";
}

void AudioCapture::onReadMicrophone(void *context)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (context != NULL)
    {
        AudioCapture *thiz = static_cast<AudioCapture*>(context);
        thiz->startThreadLoop();
        //QMetaObject::invokeMethod(thiz, "startThreadLoop", Qt::DirectConnection);
    }
    else
        qWarning() << "Can't call readMicrophone, context is NULL";
}

bool AudioCapture::setupMicrophoneStream()
{
    // FIXME: Get these parameters more dynamically from the control
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 1
    };
    int error = 0;

    m_paStream = pa_simple_new(NULL, "qtubuntu-camera", PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error);
    if (m_paStream == NULL)
    {
        qWarning() << "Failed to open a PulseAudio channel to read the microphone";
        return false;
    }

    qDebug() << "m_paStream: " << m_paStream;

    return true;
}

bool AudioCapture::setupPipe()
{
    qDebug() << __PRETTY_FUNCTION__;

    if (m_audioPipe >= 0)
    {
        qWarning() << "/dev/socket/micshm already opened, not opening twice";
        return true;
    }

    qDebug() << "Opening /dev/socket/micshm pipe";
    m_audioPipe = open("/dev/socket/micshm", O_WRONLY);
    if (m_audioPipe < 0)
    {
        qWarning() << "Failed to open audio data pipe /dev/socket/micshm: " << strerror(errno);
        return false;
    }

    qDebug() << "Opened /dev/socket/micshm pipe";

    return true;
}

int AudioCapture::writeDataToPipe()
{
    qDebug() << __PRETTY_FUNCTION__;

    // Don't open the named pipe twice
    if (m_audioPipe < 0)
    {
        if (!setupPipe())
        {
            qWarning() << "Failed to open /dev/socket/micshm, cannot write data to pipe";
            return 0;
        }
    }

    qDebug() << "m_audioPipe: " << m_audioPipe;

    int num = 0;
    num = loopWrite(m_audioPipe, m_audioBuf, sizeof(m_audioBuf));
    loopWrite(STDOUT_FILENO, m_audioBuf, sizeof(m_audioBuf));
    qDebug() << "num: " << num;
    if (num != MIC_READ_BUF_SIZE)
        qWarning() << "Failed to write " << num << " bytes to /dev/socket/micshm: " << strerror(errno) << " (" << errno << ")";
    else
        qDebug() << "Wrote " << num << " bytes to /dev/socket/micshm";

    return num;
}

ssize_t AudioCapture::loopWrite(int fd, const void *data, size_t size)
{
    ssize_t ret = 0;
    while (size > 0)
    {
        ssize_t r;
        if ((r = write(fd, data, size)) < 0)
            return r;
        if (r == 0)
            break;
        ret += r;
        //data = static_cast<const int16_t*>(data + r);
        data = (const int16_t*) data + r;
        size -= (size_t) r;
        //size -= static_cast<size_t>(r);
    }
    return ret;
}
