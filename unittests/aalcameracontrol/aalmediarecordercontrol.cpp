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

AalMediaRecorderControl::~AalMediaRecorderControl()
{
}

void AalMediaRecorderControl::applySettings()
{
    qDebug() << Q_FUNC_INFO << " is not used";
}

qint64 AalMediaRecorderControl::duration() const
{
    return m_duration;
}

bool AalMediaRecorderControl::isMuted() const
{
    return false;
}

QUrl AalMediaRecorderControl::outputLocation() const
{
    return m_outputLocation;
}

bool AalMediaRecorderControl::setOutputLocation(const QUrl &location)
{
    Q_UNUSED(location);
    return true;
}

QMediaRecorder::State AalMediaRecorderControl::state() const
{
    return m_currentState;
}

QMediaRecorder::Status AalMediaRecorderControl::status() const
{
    return m_currentStatus;
}

qreal AalMediaRecorderControl::volume() const
{
    return 1.0;
}

void AalMediaRecorderControl::init()
{
}

void AalMediaRecorderControl::deleteRecorder()
{
}

void AalMediaRecorderControl::setMuted(bool muted)
{
    Q_UNUSED(muted);
}

void AalMediaRecorderControl::setState(QMediaRecorder::State state)
{
    Q_UNUSED(state);
}

void AalMediaRecorderControl::setVolume(qreal gain)
{
    Q_UNUSED(gain);
}

void AalMediaRecorderControl::updateDuration()
{
}

void AalMediaRecorderControl::handleError()
{
    Q_EMIT error(-1, "Error on recording video");
}

void AalMediaRecorderControl::setStatus(QMediaRecorder::Status status)
{
    Q_UNUSED(status);
}

int AalMediaRecorderControl::startRecording()
{
    return 0;
}

void AalMediaRecorderControl::stopRecording()
{
}
