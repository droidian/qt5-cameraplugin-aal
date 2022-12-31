/*
 * Copyright © 2021-2022 UBports Foundation.
 *
 * Contact: Alberto Mardegan <mardy@users.sourceforge.net>
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

#ifndef VIDEO_SINK_P_H
#define VIDEO_SINK_P_H

#include "video_sink.h"

#include <functional>

typedef std::function<VideoSink *(uint32_t textureId, QObject *parent)>
    VideoSinkFactory;
typedef uint32_t PlayerKey;

VideoSinkFactory createVideoSinkFactory(PlayerKey key);

class VideoSinkPrivate
{
public:
    virtual ~VideoSinkPrivate() = default;

    QMatrix4x4 m_transformationMatrix;
};

#endif // VIDEO_SINK_P_H
