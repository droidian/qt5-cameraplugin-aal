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
#include "aalcamerafocuscontrol.h"

class tst_AalCameraFocusControl : public QObject
{
    Q_OBJECT
private slots:
    void init();
    void cleanup();

    void customPoint();
    void focusMode();
    void focusModeAfterDoubleInit();
    void focusPointMode();
    void point2Region_data();
    void point2Region();

private:
    AalCameraFocusControl *m_focusControl;
    AalCameraService *m_service;
};

void tst_AalCameraFocusControl::init()
{
    m_service = new AalCameraService();
    m_focusControl = new AalCameraFocusControl(m_service);
}

void tst_AalCameraFocusControl::cleanup()
{
    delete m_focusControl;
    delete m_service;
}

void tst_AalCameraFocusControl::customPoint()
{
    QSignalSpy spy(m_focusControl, SIGNAL(customFocusPointChanged(const QPointF &)));

    QPointF point(0.2, 0.3);
    m_focusControl->setCustomFocusPoint(point);

    QCOMPARE(m_focusControl->customFocusPoint(), point);
    QCOMPARE(spy.count(), 1);
}

void tst_AalCameraFocusControl::focusMode()
{
    QSignalSpy spy(m_focusControl, SIGNAL(focusModeChanged(QCameraFocus::FocusModes)));

    QCameraFocus::FocusModes mode = QCameraFocus::InfinityFocus;
    m_focusControl->setFocusMode(mode);

    QCOMPARE(m_focusControl->focusMode(), mode);
    QCOMPARE(spy.count(), 1);
}

void tst_AalCameraFocusControl::focusModeAfterDoubleInit()
{
    // default focusMode is AutoFocus
    QCOMPARE(m_focusControl->focusMode(), QCameraFocus::AutoFocus);

    // set focusMode to ContinuousFocus
    QSignalSpy spy(m_focusControl, SIGNAL(focusModeChanged(QCameraFocus::FocusModes)));
    QCameraFocus::FocusModes mode = QCameraFocus::InfinityFocus;
    m_focusControl->setFocusMode(mode);
    QCOMPARE(m_focusControl->focusMode(), mode);
    QCOMPARE(spy.count(), 1);

    // checking that focusMode does not change upon init()
    spy.clear();
    CameraControlListener* androidListener = new CameraControlListener;
    CameraControl* androidControl = android_camera_connect_to(BACK_FACING_CAMERA_TYPE, androidListener);
    m_focusControl->init(androidControl, androidListener);
    QCOMPARE(m_focusControl->focusMode(), mode);
    QCOMPARE(spy.count(), 0);

    // checking that focusMode does not change upon second call to init()
    // This happens for example upon switching cameras or reactivating the camera
    spy.clear();
    m_focusControl->init(androidControl, androidListener);
    QCOMPARE(m_focusControl->focusMode(), mode);
    QCOMPARE(spy.count(), 0);


    delete androidListener;
}

void tst_AalCameraFocusControl::focusPointMode()
{
    QSignalSpy spy(m_focusControl, SIGNAL(focusPointModeChanged(QCameraFocus::FocusPointMode)));

    QCameraFocus::FocusPointMode mode = QCameraFocus::FocusPointCenter;
    m_focusControl->setFocusPointMode(mode);

    QCOMPARE(m_focusControl->focusPointMode(), mode);
    QCOMPARE(spy.count(), 1);
}

void tst_AalCameraFocusControl::point2Region_data()
{
    QTest::addColumn<qreal>("x");
    QTest::addColumn<qreal>("y");
    QTest::addColumn<int>("left");
    QTest::addColumn<int>("right");
    QTest::addColumn<int>("top");
    QTest::addColumn<int>("bottom");

    QTest::newRow("center") << (qreal)0.5 << (qreal)0.5 << -100 << 100 << -100 << 100;
    QTest::newRow("topLeft") << (qreal)0.0 << (qreal)1.0 << -1000 << -800 << 800 << 1000;
    QTest::newRow("bottomRight") << (qreal)1.0 << (qreal)0.0 << 800 << 1000 << -1000 << -800;
}

void tst_AalCameraFocusControl::point2Region()
{
    QFETCH(qreal, x);
    QFETCH(qreal, y);
    QFETCH(int, left);
    QFETCH(int, right);
    QFETCH(int, top);
    QFETCH(int, bottom);

    AalCameraFocusControl focusControl(0);

    QPointF point(x, y);
    FocusRegion region;
    region = m_focusControl->point2Region(point);
    QCOMPARE(region.left, left);
    QCOMPARE(region.right, right);
    QCOMPARE(region.top, top);
    QCOMPARE(region.bottom, bottom);
}


QTEST_GUILESS_MAIN(tst_AalCameraFocusControl)

#include "tst_aalcamerafocuscontrol.moc"
