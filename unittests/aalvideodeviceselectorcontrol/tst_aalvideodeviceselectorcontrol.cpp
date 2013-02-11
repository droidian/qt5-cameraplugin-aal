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

#include <QtTest/QtTest>
#include <QSignalSpy>

#include "aalcameraservice.h"

#define private public
#include "aalvideodeviceselectorcontrol.h"

class tst_AalVideoDeviceSelectorControl : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void selectDevice();

private:
    AalVideoDeviceSelectorControl *m_selectControl;
    AalCameraService *m_service;
};

void tst_AalVideoDeviceSelectorControl::initTestCase()
{
    m_service = new AalCameraService();
    m_selectControl = new AalVideoDeviceSelectorControl(m_service);
}

void tst_AalVideoDeviceSelectorControl::cleanupTestCase()
{
    delete m_selectControl;
    delete m_service;
}

void tst_AalVideoDeviceSelectorControl::selectDevice()
{
    QSignalSpy spy(m_selectControl, SIGNAL(selectedDeviceChanged(int)));
    QSignalSpy spy2(m_selectControl, SIGNAL(selectedDeviceChanged(QString)));

    m_selectControl->setSelectedDevice(1);

    QCOMPARE(m_selectControl->selectedDevice(), 1);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy2.count(), 1);
}

QTEST_MAIN(tst_AalVideoDeviceSelectorControl)

#include "tst_aalvideodeviceselectorcontrol.moc"
