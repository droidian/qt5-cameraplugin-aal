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
 *
 * Authored by: Jim Hodapp <jim.hodapp@canonical.com>
 */

#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include <hybris/media/media_recorder_layer.h>

#include <stdint.h>

#include <QObject>

class AalMediaRecorderControl;
struct MediaRecorderWrapper;

struct pa_simple;

class AudioCapture : public QObject
{
    Q_OBJECT

    typedef void (*RecorderReadAudioCallback)(void *context);
public:
    static const int AUDIO_CAPTURE_GENERAL_ERROR = -1;
    static const int AUDIO_CAPTURE_TIMEOUT_ERROR = -2;

    explicit AudioCapture(MediaRecorderWrapper *mediaRecorder);
    ~AudioCapture();

    bool init(RecorderReadAudioCallback callback, void *context);
    /* Terminates the Pulseaudio reader/writer QThread */
    int setupMicrophoneStream();
    void stopCapture();

public Q_SLOTS:
    void run();

private:
    int readMicrophone();
    bool setupPipe();
    ssize_t loopWrite(int fd, const void *data, size_t len);
    int writeDataToPipe();

    pa_simple *m_paStream;
    int16_t m_audioBuf[MIC_READ_BUF_SIZE];

    int m_audioPipe;
    bool m_flagExit;
    MediaRecorderWrapper *m_mediaRecorder;
};

#endif // AUDIOCAPTURE_H
