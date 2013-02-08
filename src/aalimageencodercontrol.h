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
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef AALIMAGEENCODERCONTROL_H
#define AALIMAGEENCODERCONTROL_H

#include <QImageEncoderControl>
#include <QList>
#include <QString>
#include <QStringList>

class AalCameraService;
class CameraControl;

class AalImageEncoderControl : public QImageEncoderControl
{
public:
    AalImageEncoderControl(AalCameraService *service, QObject *parent = 0);
    ~AalImageEncoderControl();

    QString imageCodecDescription(const QString &codec) const;
    QImageEncoderSettings imageSettings() const;
    void setImageSettings(const QImageEncoderSettings &settings);
    QStringList supportedImageCodecs() const;
    QList<QSize> supportedResolutions(const QImageEncoderSettings &settings, bool *continuous = 0) const;

    void init(CameraControl *control);
    void setSize(const QSize &size);

    static void setPictureSizeCb(void *ctx, int width, int height);

private:
    AalCameraService *m_service;
    QList<QSize> m_availableSizes;
    QSize m_currentSize;

    void setPictureSize(int width, int height);
};

#endif
