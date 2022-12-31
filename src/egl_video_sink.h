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

#ifndef EGL_VIDEO_SINK_H
#define EGL_VIDEO_SINK_H

#include "video_sink_p.h"

struct BufferMetadata
{
    int width;
    int height;
    int fourcc;
    int stride;
    int offset;
};

struct BufferData
{
    int fd;
    BufferMetadata meta;
};

class EglVideoSinkPrivate;
class EglVideoSink: public VideoSink
{
    Q_OBJECT

public:
    static VideoSinkFactory createFactory(PlayerKey playerKey);
    virtual ~EglVideoSink();

    bool swapBuffers() override;

private:
    EglVideoSink(uint32_t gl_texture, PlayerKey key, QObject *parent);
    Q_DECLARE_PRIVATE(EglVideoSink);
};

#endif // EGL_VIDEO_SINK_H
