/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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

#ifndef MEDIA_SIGNALS_H
#define MEDIA_SIGNALS_H

#include <QImage>
#include <QMutex>
#include <QObject>
#include <QSize>

class CameraControl;

class SharedSignal : public QObject
{
    Q_OBJECT
public:

    /** This needs to stay in sync with the same definition in media-hub
     *  include/core/media/player.h
     */
    enum Orientation
    {
        rotate0,
        rotate90,
        rotate180,
        rotate270
    };

    static SharedSignal* instance();

Q_SIGNALS:
    /** Thrown by qtubuntu-media to signal to qtvideo-node when the video sink
     * has been reset. This allows qtvideo-node to take appropriate action to
     * reset some of its internal state.
     */
    void sinkReset();
    /** Thrown by qtvideo-node to signal when the GL texture has
     * been successfully created.
     */
    void textureCreated(unsigned int textureID);
    /** Thrown by qtvideo-node to signal when the GLConsumer instance
     * has been successfully passed to it and set.
     */
    void glConsumerSet();
    /** Thrown by AalMediaPlayerService to indicate to qtvideo-node
     * which rotation transformation matrix needs to be applied to
     * each video frame for rendering.
     * @param orientaiton rotate0, rotate90, rotate180, rotate270
     * @param size width/height of a video frame
     */
    void setOrientation(const SharedSignal::Orientation& orientation, const QSize &size);
    /** Thrown by AalVideoRendererControl to signal set the snapshot size.
     */
    void setSnapshotSize(const QSize &size);
    /** Thrown by ShaderVideoNode when the snapshot image has been successfully
     * taken and stored in memory.
     */
    void snapshotTaken(QImage snapshotImage);
    /** Thrown by AalVideoRendererControl to signal when to capture
     * a still image from the preview video stream.
     */
    void takeSnapshot(const CameraControl *control);

protected:
    SharedSignal(QObject *parent = NULL);

private:
    Q_DISABLE_COPY(SharedSignal);

    static SharedSignal *m_sharedSignal;
    static QMutex m_mutex;
};

#endif // SHAREDSIGNAL_H
