/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Guenter Schwann <guenter.schwann@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtTest/QtTest>
#include <QSignalSpy>

#include "aalcameraservice.h"

#define private public
#include "aalcameracontrol.h"

class tst_AalCameraControl : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void setState();
    void captureMode();

private:
    AalCameraControl *m_cameraControl;
    AalCameraService *m_service;
};

void tst_AalCameraControl::initTestCase()
{
    m_service = new AalCameraService();
    m_cameraControl = new AalCameraControl(m_service);
}

void tst_AalCameraControl::cleanupTestCase()
{
    delete m_cameraControl;
    delete m_service;
}

void tst_AalCameraControl::setState()
{
    QSignalSpy spy(m_cameraControl, SIGNAL(stateChanged(QCamera::State)));

    QCamera::State state = QCamera::ActiveState;
    m_cameraControl->setState(state);

    QCOMPARE(m_cameraControl->state(), state);
    QCOMPARE(spy.count(), 1);
}

void tst_AalCameraControl::captureMode()
{
    QSignalSpy spy(m_cameraControl, SIGNAL(captureModeChanged(QCamera::CaptureModes)));

    QCamera::CaptureModes mode = QCamera::CaptureVideo;
    m_cameraControl->setCaptureMode(mode);

    QCOMPARE(m_cameraControl->captureMode(), mode);
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(tst_AalCameraControl)

#include "tst_aalcameracontrol.moc"
