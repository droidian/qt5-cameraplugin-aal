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

#include "aalcameraservice.h"

#define private public
#include "aalimagecapturecontrol.h"

class tst_AalImageCaptureControl : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void chooseOptimalSize16by9();
    void chooseOptimalSizeEmpty();

private:
    AalImageCaptureControl *m_icControl;
    AalCameraService *m_service;

    friend class AalImageCaptureControl;
};

void tst_AalImageCaptureControl::initTestCase()
{
    m_service = new AalCameraService();
    m_icControl = new AalImageCaptureControl(m_service);
}

void tst_AalImageCaptureControl::cleanupTestCase()
{
    delete m_icControl;
    delete m_service;
}

void tst_AalImageCaptureControl::chooseOptimalSize16by9()
{
    Q_ASSERT(m_service->isBackCameraUsed());

    m_icControl->m_aspectRatio = (float)16 / (float)9;
    QList<QSize> resolutions;
    resolutions.append(QSize(1920, 1080));
    resolutions.append(QSize(1280, 720));
    resolutions.append(QSize(960, 720));

    QCOMPARE(m_icControl->chooseOptimalSize(resolutions), QSize(1920, 1080));
}

void tst_AalImageCaptureControl::chooseOptimalSizeEmpty()
{
    m_icControl->m_aspectRatio = (float)4 / (float)3;
    QList<QSize> resolutions;

    QCOMPARE(m_icControl->chooseOptimalSize(resolutions), QSize());
}

QTEST_MAIN(tst_AalImageCaptureControl)

#include "tst_aalimagecapturecontrol.moc"
