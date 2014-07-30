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
#include "aalcameraexposurecontrol.h"

class tst_AalCameraExposureControl : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();

    void setUnsupportedParameter();
    void setExposureMode();

private:
    AalCameraExposureControl *m_exposureControl;
    AalCameraService *m_service;
};

void tst_AalCameraExposureControl::initTestCase()
{
    m_service = new AalCameraService();
    m_exposureControl = m_service->exposureControl();
    m_service->connectCamera();
}

void tst_AalCameraExposureControl::cleanupTestCase()
{
    delete m_service;
}

void tst_AalCameraExposureControl::setUnsupportedParameter()
{
    QCameraExposureControl::ExposureParameter parameter = QCameraExposureControl::ISO;

    QSignalSpy spyActual(m_exposureControl, SIGNAL(actualValueChanged(int)));
    QSignalSpy spyRequested(m_exposureControl, SIGNAL(requestedValueChanged(int)));

    bool supported = m_exposureControl->isParameterSupported(parameter);
    bool valid = m_exposureControl->setValue(parameter, QVariant::fromValue(200));
    QVariant requestedValue = m_exposureControl->requestedValue(parameter);
    QVariant actualValue = m_exposureControl->actualValue(parameter);

    QVERIFY(!supported);
    QVERIFY(!valid);
    QCOMPARE(requestedValue, QVariant());
    QCOMPARE(actualValue, QVariant());

    QCOMPARE(spyActual.count(), 0);
    QCOMPARE(spyRequested.count(), 0);
}

void tst_AalCameraExposureControl::setExposureMode()
{
    QCameraExposureControl::ExposureParameter parameter = QCameraExposureControl::ExposureMode;

    QSignalSpy spyActual(m_exposureControl, SIGNAL(actualValueChanged(int)));
    QSignalSpy spyRequested(m_exposureControl, SIGNAL(requestedValueChanged(int)));

    bool supported = m_exposureControl->isParameterSupported(parameter);
    bool valid = m_exposureControl->setValue(parameter, QVariant::fromValue(QCameraExposure::ExposureSports));
    QVariant requestedValue = m_exposureControl->requestedValue(parameter);
    QVariant actualValue = m_exposureControl->actualValue(parameter);

    QVERIFY(supported);
    QVERIFY(valid);
    QCOMPARE(requestedValue, QVariant::fromValue(QCameraExposure::ExposureSports));
    QCOMPARE(actualValue, QVariant::fromValue(QCameraExposure::ExposureSports));

    QCOMPARE(spyActual.count(), 1);
    QCOMPARE(spyRequested.count(), 1);
}

QTEST_MAIN(tst_AalCameraExposureControl)

#include "tst_aalcameraexposurecontrol.moc"
