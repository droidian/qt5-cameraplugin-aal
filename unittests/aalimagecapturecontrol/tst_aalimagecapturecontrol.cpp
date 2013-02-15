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
#include <cmath>

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

    void priorityAspectRatio();

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

    m_icControl->m_aspectRatio = 16.0f / 9.0f;
    QList<QSize> resolutions;
    resolutions.append(QSize(1920, 1080));
    resolutions.append(QSize(1280, 720));
    resolutions.append(QSize(960, 720));

    QCOMPARE(m_icControl->chooseOptimalSize(resolutions), QSize(1920, 1080));
}

void tst_AalImageCaptureControl::chooseOptimalSizeEmpty()
{
    m_icControl->m_aspectRatio = 4.0f / 3.0f;
    QList<QSize> resolutions;

    QCOMPARE(m_icControl->chooseOptimalSize(resolutions), QSize());
}

void tst_AalImageCaptureControl::priorityAspectRatio()
{
    Q_ASSERT(m_service->isBackCameraUsed());

    QList<float> backAspectRatios;
    backAspectRatios.append(16.0f / 9.0f);
    backAspectRatios.append(15.0f / 10.0f);
    backAspectRatios.append(4.0f / 3.0f);
    backAspectRatios.append(5.0f / 4.0f);
    m_icControl->getPriorityAspectRatios();
    for (uint8_t i=0; i<4; ++i) {
        const float EPSILON = 10e-2;
        qDebug() << "m_icControl->m_prioritizedAspectRatios[i]: " << m_icControl->m_prioritizedAspectRatios[i] << endl;
        qDebug() << "backAspectRatios[i]: " << backAspectRatios[i] << endl;
        Q_ASSERT(fabs(m_icControl->m_prioritizedAspectRatios[i] - backAspectRatios[i]) < EPSILON);
    }
}

QTEST_MAIN(tst_AalImageCaptureControl)

#include "tst_aalimagecapturecontrol.moc"
