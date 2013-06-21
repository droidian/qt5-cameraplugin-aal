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
#include "aalviewfindersettingscontrol.h"

#include <hybris/camera/camera_compatibility_layer_capabilities.h>

const QSize AalVideoEncoderSettingsControl::DEFAULT_SIZE = QSize(1280,720);
const int AalVideoEncoderSettingsControl::DEFAULT_FPS = 30;

/*!
 * \brief AalVideoEncoderSettingsControl::AalVideoEncoderSettingsControl
 * \param service
 * \param parent
 */
AalVideoEncoderSettingsControl::AalVideoEncoderSettingsControl(AalCameraService *service, QObject *parent)
    : QVideoEncoderSettingsControl(parent),
      m_service(service)
{
}

/*!
 * \reimp
 */
void AalVideoEncoderSettingsControl::setVideoSettings(const QVideoEncoderSettings &settings)
{
    bool continuous;
    if (supportedVideoCodecs().contains(settings.codec()))
        m_settings.setCodec(settings.codec());

    m_settings.setBitRate(settings.bitRate());

    if (supportedFrameRates(settings, &continuous).contains(settings.frameRate()))
        m_settings.setFrameRate(settings.frameRate());

    if (supportedResolutions(settings, &continuous).contains(settings.resolution()))
        m_settings.setResolution(settings.resolution());

    // FIXME support more options
}

/*!
 * \reimp
 */
QList<qreal> AalVideoEncoderSettingsControl::supportedFrameRates(const QVideoEncoderSettings &settings, bool *continuous) const
{
    Q_UNUSED(settings);
    Q_UNUSED(continuous);
    // FIXME get data from android
    QList<qreal> fps;
    fps << 15 << 30;
    return fps;
}

/*!
 * \reimp
 */
QList<QSize> AalVideoEncoderSettingsControl::supportedResolutions(const QVideoEncoderSettings &settings, bool *continuous) const
{
    Q_UNUSED(settings);
    Q_UNUSED(continuous);

    if (m_availableSizes.isEmpty())
        querySupportedResolution();

    return m_availableSizes;
}

/*!
 * \reimp
 */
QStringList AalVideoEncoderSettingsControl::supportedVideoCodecs() const
{
    // FIXME get data from android
    QStringList codecs;
    codecs << QString("H.264");
    return codecs;
}

/*!
 * \reimp
 */
QString AalVideoEncoderSettingsControl::videoCodecDescription(const QString &codec) const
{
    return codec;
}

/*!
 * \reimp
 */
QVideoEncoderSettings AalVideoEncoderSettingsControl::videoSettings() const
{
    return m_settings;
}

/*!
 * \brief AalMediaRecorderControl::getAspectRatio returns the curent used aspect ratio
 * \return
 */
float AalVideoEncoderSettingsControl::getAspectRatio() const
{
    QSize resolution = m_settings.resolution();
    return (float)resolution.width() / (float)resolution.height();
}

/*!
 * \brief AalVideoEncoderSettingsControl::init
 * \param control
 * \param listener
 */
void AalVideoEncoderSettingsControl::init(CameraControl *control, CameraControlListener *listener)
{
    Q_UNUSED(control);
    Q_UNUSED(listener);

    if (m_availableSizes.isEmpty())
        querySupportedResolution();

    if (!m_availableSizes.contains(m_settings.resolution()) && !m_availableSizes.empty()) {
        m_settings.setResolution(m_availableSizes[0]);
    }
}

/*!
 * \brief AalVideoEncoderSettingsControl::resetAllSettings
 */
void AalVideoEncoderSettingsControl::resetAllSettings()
{
    m_availableSizes.clear();

    int videoBitRate = 7 * DEFAULT_SIZE.width() * DEFAULT_SIZE.height();
    m_settings.setBitRate(videoBitRate);
    m_settings.setCodec(QString("H.264"));
    m_settings.setFrameRate(DEFAULT_FPS);
    m_settings.setResolution(DEFAULT_SIZE.width(), DEFAULT_SIZE.height());
}

/*!
 * \brief AalVideoEncoderSettingsControl::querySupportedResolution gets the
 * supported resolutions from android, and saves the to the m_availableSizes member
 */
void AalVideoEncoderSettingsControl::querySupportedResolution() const
{
    CameraControl *cc = m_service->androidControl();
    if (!cc)
        return;


    AalVideoEncoderSettingsControl *vSettings = const_cast<AalVideoEncoderSettingsControl*>(this);
    android_camera_enumerate_supported_video_sizes(cc, &AalVideoEncoderSettingsControl::sizeCB,
                                                   vSettings);

    if (m_availableSizes.isEmpty()) {
        // android devices where video and viewfinder are "linked", no sizes are returned
        // so use the viewfinder sizes
        m_availableSizes = m_service->viewfinderControl()->supportedSizes();
    }
}

/*!
 * \brief AalVideoEncoderSettingsControl::sizeCB calback function to get the supported sizes
 * \param ctx pointer to the AalVideoEncoderSettingsControl object that requests the sizes
 * \param width
 * \param height
 */
void AalVideoEncoderSettingsControl::sizeCB(void *ctx, int width, int height)
{
    AalVideoEncoderSettingsControl *self = (AalVideoEncoderSettingsControl*)ctx;
    self->m_availableSizes.append(QSize(width, height));
}
