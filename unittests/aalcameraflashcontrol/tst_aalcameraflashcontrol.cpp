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

#include "aalcameraservice.h"

#define private public
#include "aalcameraflashcontrol.h"

class tst_AalCameraFlashControl : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void flashMode();

private:
    AalCameraFlashControl *m_flashControl;
    AalCameraService *m_service;
};

void tst_AalCameraFlashControl::initTestCase()
{
    m_service = new AalCameraService();
    m_flashControl = new AalCameraFlashControl(m_service);
}

void tst_AalCameraFlashControl::cleanupTestCase()
{
    delete m_flashControl;
    delete m_service;
}

void tst_AalCameraFlashControl::flashMode()
{
    QCameraExposure::FlashModes mode = QCameraExposure::FlashOn;
    m_flashControl->setFlashMode(mode);

    QCOMPARE(m_flashControl->flashMode(), mode);
}

QTEST_MAIN(tst_AalCameraFlashControl)

#include "tst_aalcameraflashcontrol.moc"
