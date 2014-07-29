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
      m_audioPipe(-1),
      m_flagExit(false),
      m_mediaRecorder(mediaRecorder)
{
    qDebug() << "Instantiating new AudioCapture instance";
}

AudioCapture::~AudioCapture()
{
    if (m_audioPipe >= 0)
        close(m_audioPipe);
    if (m_paStream != NULL)
        pa_simple_free(m_paStream);
}

bool AudioCapture::init(StartWorkerThreadCb cb, void *context)
{
    qDebug() << "Set android_recorder_set_audio_read_cb";
    // The MediaRecorderLayer will call method (cb) when it's ready to encode a new audio buffer
    android_recorder_set_audio_read_cb(m_mediaRecorder, cb, context);

    return true;
}

void AudioCapture::stopCapture()
{
    qDebug() << __PRETTY_FUNCTION__;
    m_flagExit = true;
}

void AudioCapture::run()
{
    qDebug() << __PRETTY_FUNCTION__;

    int bytesWritten = 0, bytesRead = 0;
    const size_t readSize = sizeof(m_audioBuf);

    if (!setupMicrophoneStream())
    {
        qWarning() << "Failed to setup PulseAudio microphone recording stream";
        return;
    }

    if (!setupPipe())
    {
        qWarning() << "Failed to open /dev/socket/micshm, cannot write data to pipe";
        return;
    }

    do {
        qDebug() << "--> reading from the mic";
        bytesRead = readMicrophone();
        if (bytesRead > 0)
        {
            qDebug() << "--> writing to the pipe";
            bytesWritten = writeDataToPipe();
        }
    } while (bytesRead == readSize
                && bytesWritten == readSize
                && !m_flagExit);

    qWarning() << "Broke out of the AudioCapture thread loop, exiting thread loop";
}

int AudioCapture::readMicrophone()
{
    qDebug() << __PRETTY_FUNCTION__;

    int ret = 0, error = 0;
    const size_t readSize = sizeof(m_audioBuf);
    // Reinitialize the audio buffer
    //std::fill_n(m_audioBuf, (sizeof(m_audioBuf) - sizeof(int16_t)), 0);
    qDebug() << "Reading microphone data...";
    qDebug() << "m_paStream: " << m_paStream << ", m_audioBuf: " << m_audioBuf << ", sizeof: " << readSize;
    ret = pa_simple_read(m_paStream, m_audioBuf, readSize, &error);
    if (ret < 0)
    {
        qWarning() << "Failed to read audio from the microphone: " << pa_strerror(error);
        goto exit;
    }
    else
        ret = readSize;

    qDebug() << "Read in " << ret << " bytes";

exit:
    return ret;
}

void AudioCapture::startThreadLoop()
{
    qDebug() << __PRETTY_FUNCTION__;
    Q_EMIT startThread();
    qDebug() << "Emitted startThread()";
}

void AudioCapture::onReadMicrophone(void *context)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (context != NULL)
    {
        AudioCapture *thiz = static_cast<AudioCapture*>(context);
        thiz->startThreadLoop();
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

    qDebug() << "Opened /dev/socket/micshm pipe on fd: " << m_audioPipe;

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

    int num = 0;
    const size_t writeSize = sizeof(m_audioBuf);
    num = loopWrite(m_audioPipe, m_audioBuf, writeSize);
    if (num != writeSize)
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
        data = (const int16_t*) data + r;
        size -= (size_t) r;
    }
    return ret;
}
