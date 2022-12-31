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

#include "hybris_video_sink.h"

#include <QMutex>
#include <QMutexLocker>

#include <hybris/media/media_codec_layer.h>
#include <hybris/media/surface_texture_client_hybris.h>

class HybrisVideoSinkPrivate: public VideoSinkPrivate
{
    friend class HybrisVideoSink;

    static void on_frame_available_callback(GLConsumerWrapperHybris, void* context)
    {
        if (not context)
            return;

        auto thiz = static_cast<HybrisVideoSink*>(context);
        thiz->onFrameAvailable();
    }

public:
    HybrisVideoSinkPrivate(uint32_t gl_texture, HybrisVideoSink *q):
        gl_texture{gl_texture},
        graphics_buffer_consumer{decoding_service_get_igraphicbufferconsumer()},
        gl_texture_consumer{gl_consumer_create_by_id_with_igbc(gl_texture, graphics_buffer_consumer)}
    {
        if (not graphics_buffer_consumer) {
            qCritical("video::HybrisGlSink: Could not connect to remote buffer queue.");
            return;
        };

        if (not gl_texture_consumer) {
            qCritical("video::HybrisGlSink: Could not associate local texture id with remote buffer streak.");
            return;
        };

        gl_consumer_set_frame_available_cb(gl_texture_consumer,
                                           HybrisVideoSinkPrivate::on_frame_available_callback,
                                           q);

    }

    ~HybrisVideoSinkPrivate()
    {
        gl_consumer_set_frame_available_cb(gl_texture_consumer,
                                           HybrisVideoSinkPrivate::on_frame_available_callback,
                                           nullptr);
    }

    void updateTransformationMatrix() {
        // TODO: The underlying API really should tell us if everything is ok.
        gl_consumer_get_transformation_matrix(gl_texture_consumer,
                                              m_transformationMatrix.data());
    }

    uint32_t gl_texture;
    IGBCWrapperHybris graphics_buffer_consumer;
    GLConsumerWrapperHybris gl_texture_consumer;
};

HybrisVideoSink::HybrisVideoSink(uint32_t gl_texture,
                                 QObject *parent):
    VideoSink(new HybrisVideoSinkPrivate(gl_texture, this), parent)
{
}

HybrisVideoSink::~HybrisVideoSink()
{
}

void HybrisVideoSink::onFrameAvailable()
{
    Q_D(HybrisVideoSink);
    d->updateTransformationMatrix();
    Q_EMIT frameAvailable();
}

VideoSinkFactory HybrisVideoSink::createFactory(PlayerKey key)
{
    // It's okay-ish to use static map here. Point being that we currently have no way
    // of terminating the session with the decoding service anyway.
    static QHash<PlayerKey, DSSessionWrapperHybris> lut;
    static QMutex lut_guard;

    // Scoping access to the lut to ensure that the lock is kept for as short as possible.
    {
        QMutexLocker lg(&lut_guard);
        if (!lut.contains(key))
            lut[key] = decoding_service_create_session(key);
    }

    return [](uint32_t textureId, QObject *parent) {
        return new HybrisVideoSink(textureId, parent);
    };
}

bool HybrisVideoSink::swapBuffers()
{
    Q_D(HybrisVideoSink);

    // TODO: The underlying API really should tell us if everything is ok.
    gl_consumer_update_texture(d->gl_texture_consumer);
    return true;
}
