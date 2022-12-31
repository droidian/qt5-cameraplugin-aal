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

#include "video_sink_p.h"
#include "egl_video_sink.h"

VideoSink::VideoSink(VideoSinkPrivate *d, QObject *parent):
    QObject(parent),
    d_ptr(d)
{
}

VideoSink::~VideoSink() = default;

const QMatrix4x4 &VideoSink::transformationMatrix() const
{
    Q_D(const VideoSink);
    return d->m_transformationMatrix;
}

VideoSinkFactory createVideoSinkFactory(
    PlayerKey key)
{
    return EglVideoSink::createFactory(key);
}
