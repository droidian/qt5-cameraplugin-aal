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

#include <QFile>
#include <QtTest/QtTest>
#include <QUrl>

#include "aalcameraservice.h"

#define private public
#include "aalmediarecordercontrol.h"

class tst_AalMediaRecorderControl : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void setState();

private:
    AalMediaRecorderControl *m_recorderControl;
    AalCameraService *m_service;
};

void tst_AalMediaRecorderControl::initTestCase()
{
    m_service = new AalCameraService();
    m_recorderControl = new AalMediaRecorderControl(m_service);
    m_service->connectCamera();
}

void tst_AalMediaRecorderControl::cleanupTestCase()
{
    delete m_recorderControl;
    delete m_service;
}

void tst_AalMediaRecorderControl::setState()
{
    QString fileName("/tmp/videotest.avi");
    QFile::remove(fileName);
    m_recorderControl->setOutputLocation(QUrl(fileName));

    QCOMPARE(m_recorderControl->state(), QMediaRecorder::StoppedState);
    QCOMPARE(m_recorderControl->status(), QMediaRecorder::UnloadedStatus);

    m_recorderControl->setState(QMediaRecorder::RecordingState);
    QCOMPARE(m_recorderControl->state(), QMediaRecorder::RecordingState);
    QCOMPARE(m_recorderControl->status(), QMediaRecorder::RecordingStatus);

    m_recorderControl->setState(QMediaRecorder::PausedState);
    QCOMPARE(m_recorderControl->state(), QMediaRecorder::RecordingState);
    QCOMPARE(m_recorderControl->status(), QMediaRecorder::RecordingStatus);

    m_recorderControl->setState(QMediaRecorder::StoppedState);
    QCOMPARE(m_recorderControl->state(), QMediaRecorder::StoppedState);
    QCOMPARE(m_recorderControl->status(), QMediaRecorder::LoadedStatus);
}

QTEST_MAIN(tst_AalMediaRecorderControl)

#include "tst_aalmediarecordercontrol.moc"
