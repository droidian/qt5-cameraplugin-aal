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

#include "aalvideoencodersettingscontrol.h"
#include "aalcameraservice.h"

const QSize AalVideoEncoderSettingsControl::DEFAULT_SIZE = QSize(1280,720);
const int AalVideoEncoderSettingsControl::DEFAULT_FPS = 30;

AalVideoEncoderSettingsControl::AalVideoEncoderSettingsControl(AalCameraService *service, QObject *parent)
    : QVideoEncoderSettingsControl(parent),
      m_service(service)
{
}

void AalVideoEncoderSettingsControl::setVideoSettings(const QVideoEncoderSettings &settings)
{
    Q_UNUSED(settings);
}

QList<qreal> AalVideoEncoderSettingsControl::supportedFrameRates(const QVideoEncoderSettings &settings, bool *continuous) const
{
    Q_UNUSED(settings);
    Q_UNUSED(continuous);
    QList<qreal> fps;
    fps << 30;
    return fps;
}

QList<QSize> AalVideoEncoderSettingsControl::supportedResolutions(const QVideoEncoderSettings &settings, bool *continuous) const
{
    Q_UNUSED(settings);
    Q_UNUSED(continuous);
    return m_availableSizes;
}

QStringList AalVideoEncoderSettingsControl::supportedVideoCodecs() const
{
    QStringList codecs;
    codecs << QString("H.264");
    return codecs;
}

QString AalVideoEncoderSettingsControl::videoCodecDescription(const QString &codec) const
{
    return codec;
}

QVideoEncoderSettings AalVideoEncoderSettingsControl::videoSettings() const
{
    return m_settings;
}

float AalVideoEncoderSettingsControl::getAspectRatio() const
{
    return 16.0 / 9.0;
}

void AalVideoEncoderSettingsControl::resetAllSettings()
{
}
