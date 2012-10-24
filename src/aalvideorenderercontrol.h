/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Guenter Schwann <guenter.schwann@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
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

#ifndef AALVIDEORENDERERCONTROL_H
#define AALVIDEORENDERERCONTROL_H

#include <QVideoRendererControl>

class AalCameraService;
class AalGLTextureBuffer;

class AalVideoRendererControl : public QVideoRendererControl
{
Q_OBJECT
public:
    AalVideoRendererControl(AalCameraService *service, QObject *parent = 0);
    ~AalVideoRendererControl();

    QAbstractVideoSurface *surface() const;
    void setSurface(QAbstractVideoSurface *surface);

    static void updateViewfinderFrameCB(void *context);

public Q_SLOTS:
    void startPreview();

Q_SIGNALS:
    void surfaceChanged(QAbstractVideoSurface *surface);

private Q_SLOTS:
    void updateViewfinderFrame();
    void getTextureId();

private:
    QAbstractVideoSurface *m_surface;
    AalCameraService *m_service;
    AalGLTextureBuffer *m_textureBuffer;

    int m_viewFinderWidth;
    int m_viewFinderHeight;
    bool m_viewFinderRunning;
};

#endif
