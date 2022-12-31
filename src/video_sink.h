/*
 * Copyright © 2021-2022 UBports Foundation.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VIDEO_SINK_H
#define VIDEO_SINK_H

#include <QMatrix4x4>
#include <QObject>
#include <QScopedPointer>

class VideoSinkPrivate;

/** @brief A video sink abstracts a queue of buffers, that receives
  * a stream of decoded video buffers from an arbitrary source.
  */
class VideoSink: public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VideoSink)

public:
    virtual ~VideoSink();

    /**
     * @brief Queries the 4x4 transformation matrix for the current frame.
     */
    const QMatrix4x4 &transformationMatrix() const;

    /**
     * @brief Releases the current buffer, and consumes the next buffer in the queue,
     * making it available for consumption by consumers of this API in an
     * implementation-specific way. Clients will usually rely on a GL texture
     * to receive the latest buffer.
     */
    virtual bool swapBuffers() = 0;

    /**
     * @brief The signal is emitted whenever a new frame is available and a subsequent
     * call to swapBuffers() will not block and return true.
     */
Q_SIGNALS:
    void frameAvailable();

protected:
    VideoSink(VideoSinkPrivate *d, QObject *parent = nullptr);

    Q_DECLARE_PRIVATE(VideoSink)
    QScopedPointer<VideoSinkPrivate> d_ptr;
};

#endif // VIDEO_SINK_H
