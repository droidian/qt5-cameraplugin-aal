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

AudioCapture::AudioCapture(MediaRecorderWrapper *mediaRecorder)
    : m_paStream(NULL),
      m_mediaRecorder(mediaRecorder),
      m_audioPipe(0)
{
    qDebug() << "Instantiating new AudioCapture instance";
    //m_audioBuf = new uint8_t[MIC_READ_BUF_SIZE];
    qDebug() << "m_audioBuf: " << m_audioBuf;

    if (!setupMicrophoneStream())
        qWarning() << "Failed to setup PulseAudio microphone recording stream";

    if (!setupPipe())
        qWarning() << "Failed to set up named pipe to transfer microphone data to the recorder";
}

AudioCapture::~AudioCapture()
{
    if (m_audioPipe > 0)
        close(m_audioPipe);
    //delete m_audioBuf;
    if (m_paStream != NULL)
        pa_simple_free(m_paStream);
}

void AudioCapture::init()
{
    // The MediaRecorderLayer will call method (onMicrophoneRead) when it's ready to encode a new audio buffer
    android_recorder_set_audio_read_cb(m_mediaRecorder, &AudioCapture::onReadMicrophone, this);
}

void AudioCapture::readMicrophone()
{
    qDebug() << __PRETTY_FUNCTION__;
    int ret = 0, error = 0;
    // Reinitialize the audio buffer
    std::fill_n(m_audioBuf, sizeof(m_audioBuf), 0);
    qDebug() << "Reading microphone data...";
    ret = pa_simple_read(m_paStream, m_audioBuf, sizeof(m_audioBuf), &error);
    if (ret < 0)
    {
        qWarning() << "Failed to read audio from the microphone";
        return;
    }
    qDebug() << "Read in " << sizeof(m_audioBuf) << " bytes";
    writeDataToPipe();
}

void AudioCapture::onReadMicrophone(void *context)
{
    qDebug() << __PRETTY_FUNCTION__;
    if (context != NULL)
    {
        AudioCapture *thiz = static_cast<AudioCapture*>(context);
        thiz->readMicrophone();
        //QMetaObject::invokeMethod(thiz, "readMicrophone", Qt::AutoConnection);
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
        .channels = 2
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
    qDebug() << "Opening /android/micshm pipe";
    //int ret = mkfifo("/tmp/fifo", S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
    m_audioPipe = open("/android/micshm", O_WRONLY);
    if (m_audioPipe < 0)
    {
        qWarning() << "Failed to open audio data pipe /android/micshm";
        return false;
    }

    qDebug() << "Opened /android/micshm pipe";

    return true;
}

void AudioCapture::writeDataToPipe()
{
    int num = 0;
    uint8_t buf[18] = { "Hello from test!\n" };
    // TODO: Consider a retry loop here in case of error?
    num = loopWrite(m_audioPipe, m_audioBuf, sizeof(m_audioBuf));
    loopWrite(STDOUT_FILENO, buf, sizeof(buf));
    qDebug() << "num: " << num;
    if (num != MIC_READ_BUF_SIZE)
        qWarning() << "Failed to write " << num << " bytes to /android/micshm: " << strerror(errno);
    else
        qDebug() << "Wrote " << num << " bytes to /android/micshm";
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
        //data = static_cast<const uint8_t*>(data + r);
        data = (const uint8_t*) data + r;
        size -= (size_t) r;
        //size -= static_cast<size_t>(r);
    }
    return ret;
}
