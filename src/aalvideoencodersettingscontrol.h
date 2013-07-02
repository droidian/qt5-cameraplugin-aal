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

#ifndef AALVIDEOENCODERSETTINGSCONTROL_H
#define AALVIDEOENCODERSETTINGSCONTROL_H

#include <QVideoEncoderSettingsControl>

class AalCameraService;
struct CameraControl;
struct CameraControlListener;

class AalVideoEncoderSettingsControl : public QVideoEncoderSettingsControl
{
    Q_OBJECT
public:
    explicit AalVideoEncoderSettingsControl(AalCameraService *service, QObject *parent = 0);

    virtual void setVideoSettings(const QVideoEncoderSettings &settings);
    virtual QList<qreal> supportedFrameRates(const QVideoEncoderSettings &settings, bool *continuous = 0) const;
    virtual QList<QSize> supportedResolutions(const QVideoEncoderSettings &settings, bool *continuous = 0) const;
    virtual QStringList	supportedVideoCodecs() const;
    virtual QString	videoCodecDescription(const QString &codec) const;
    virtual QVideoEncoderSettings videoSettings() const;
    
    float getAspectRatio() const;

    void init(CameraControl *control, CameraControlListener *listener);
    void resetAllSettings();

    static void sizeCB(void *ctx, int width, int height);

private:
    void querySupportedResolution() const;

    AalCameraService *m_service;
    QVideoEncoderSettings m_settings;
    mutable QList<QSize> m_availableSizes;

    static const QSize DEFAULT_SIZE;
    static const int DEFAULT_FPS;
    static const QString DEFAULT_CODEC;
};

#endif // AALVIDEOENCODERSETTINGSCONTROL_H
