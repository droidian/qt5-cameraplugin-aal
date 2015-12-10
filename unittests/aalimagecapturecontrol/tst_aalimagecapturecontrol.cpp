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
#include "data_validjpeg.h"
#include "data_noexifjpeg.h"

#define private public
#include "aalimagecapturecontrol.h"

class tst_AalImageCaptureControl : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void updateEXIF();

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

void tst_AalImageCaptureControl::updateEXIF()
{
    bool result;
    QTemporaryFile tmp;
    char *invalidJPEG = "INVALID_IMAGE";
    result = m_icControl->updateJpegMetadata(0, 0, &tmp);
    QCOMPARE(result, false);
    result = m_icControl->updateJpegMetadata(invalidJPEG, strlen(invalidJPEG), 0);
    QCOMPARE(result, false);
    result = m_icControl->updateJpegMetadata(invalidJPEG, strlen(invalidJPEG), &tmp);
    QCOMPARE(result, false);
    result = m_icControl->updateJpegMetadata(data_validjpeg, data_validjpeg_len, &tmp);
    QCOMPARE(result, true);
    result = m_icControl->updateJpegMetadata(data_noexifjpeg, data_noexifjpeg_len, &tmp);
    QCOMPARE(result, true);
}

QTEST_GUILESS_MAIN(tst_AalImageCaptureControl)

#include "tst_aalimagecapturecontrol.moc"
