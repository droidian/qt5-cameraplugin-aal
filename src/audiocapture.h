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
public:
    explicit AudioCapture(MediaRecorderWrapper *mediaRecorder);
    ~AudioCapture();

    void init();

    bool readyForCapture() const;

private Q_SLOTS:
    void readMicrophone();

private:
    static void onReadMicrophone(void *context);
    bool setupMicrophoneStream();
    bool setupPipe();
    ssize_t loopWrite(int fd, const void *data, size_t len);
    void writeDataToPipe();

    pa_simple *m_paStream;
    uint8_t m_audioBuf[MIC_READ_BUF_SIZE];

    MediaRecorderWrapper *m_mediaRecorder;

    int m_audioPipe;
};

#endif // AUDIOCAPTURE_H
