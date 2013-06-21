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

#ifndef AALVIEWFINDERSETTINGSCONTROL_H
#define AALVIEWFINDERSETTINGSCONTROL_H

#include <QCameraViewfinderSettingsControl>
#include <QList>
#include <QSize>

class AalCameraService;
struct CameraControl;
struct CameraControlListener;

class AalViewfinderSettingsControl : public QCameraViewfinderSettingsControl
{
public:
    AalViewfinderSettingsControl(AalCameraService *service, QObject *parent = 0);
    ~AalViewfinderSettingsControl();

    bool isViewfinderParameterSupported(ViewfinderParameter parameter) const ;
    void setViewfinderParameter(ViewfinderParameter parameter, const QVariant & value);
    QVariant viewfinderParameter(ViewfinderParameter parameter) const;

    void setSize(const QSize &size);
    QSize currentSize() const;
    const QList<QSize> &supportedSizes() const;

    void setAspectRatio(float ratio);

    void init(CameraControl *control, CameraControlListener *listener);
    void resetAllSettings();

    static void sizeCB(void* ctx, int width, int height);

private:
    QSize chooseOptimalSize(const QList<QSize> &sizes) const;

    AalCameraService *m_service;
    QSize m_currentSize;
    float m_aspectRatio;
    int m_currentFPS;
    mutable QList<QSize> m_availableSizes;
    int m_minFPS;
    int m_maxFPS;
};

#endif // AALVIEWFINDERSETTINGSCONTROL_H
