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

#include <QImage>
#include <QVideoRendererControl>
#include <qgl.h>


class AalCameraService;
struct CameraControl;
struct CameraControlListener;
class SnapshotGenerator;

class AalVideoRendererControl : public QVideoRendererControl
{
Q_OBJECT
public:
    AalVideoRendererControl(AalCameraService *service, QObject *parent = 0);
    ~AalVideoRendererControl();

    QAbstractVideoSurface *surface() const;
    void setSurface(QAbstractVideoSurface *surface);

    static void updateViewfinderFrameCB(void *context);

    const QImage &preview() const;
    void createPreview();

    bool isViewfinderRunning() const;

public Q_SLOTS:
    void init(CameraControl *control, CameraControlListener *listener);
    void startPreview();
    void stopPreview();

Q_SIGNALS:
    void surfaceChanged(QAbstractVideoSurface *surface);

private Q_SLOTS:
    void updateViewfinderFrame();
    void doStartPreview();

private:
    QAbstractVideoSurface *m_surface;
    AalCameraService *m_service;

    int m_viewFinderWidth;
    int m_viewFinderHeight;
    bool m_viewFinderRunning;
    GLuint m_textureId;
    QImage m_preview;
    SnapshotGenerator *m_snapshotGenerator;
};

#endif
