/*
 * Copyright © 2013 Canonical Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "media_recorder_layer.h"
#include "camera_control.h"

#include <qglobal.h>

class MediaRecorderListenerWrapper
{
public:
    MediaRecorderListenerWrapper()
    {
    }
};

struct MediaRecorderWrapper
{
public:
    MediaRecorderWrapper()
    {
    }
};

void android_recorder_set_error_cb(MediaRecorderWrapper *mr, on_recorder_msg_error cb,
                                   void *context)
{
    Q_UNUSED(mr);
    Q_UNUSED(cb);
    Q_UNUSED(context);
}

MediaRecorderWrapper *android_media_new_recorder()
{
    MediaRecorderWrapper *mr = new MediaRecorderWrapper;
    return mr;
}

int android_recorder_initCheck(MediaRecorderWrapper *mr)
{
    Q_UNUSED(mr);
    return 0;
}

int android_recorder_setCamera(MediaRecorderWrapper *mr, CameraControl* control)
{
    Q_UNUSED(mr);
    Q_UNUSED(control);
    return 0;
}

int android_recorder_setVideoSource(MediaRecorderWrapper *mr, VideoSource vs)
{
    Q_UNUSED(mr);
    Q_UNUSED(vs);
    return 0;
}

int android_recorder_setAudioSource(MediaRecorderWrapper *mr, AudioSource as)
{
    Q_UNUSED(mr);
    Q_UNUSED(as);
    return 0;
}

int android_recorder_setOutputFormat(MediaRecorderWrapper *mr, OutputFormat of)
{
    Q_UNUSED(mr);
    Q_UNUSED(of);
    return 0;
}

int android_recorder_setVideoEncoder(MediaRecorderWrapper *mr, VideoEncoder ve)
{
    Q_UNUSED(mr);
    Q_UNUSED(ve);
    return 0;
}

int android_recorder_setAudioEncoder(MediaRecorderWrapper *mr, AudioEncoder ae)
{
    Q_UNUSED(mr);
    Q_UNUSED(ae);
    return 0;
}

int android_recorder_setOutputFile(MediaRecorderWrapper *mr, int fd)
{
    Q_UNUSED(mr);
    Q_UNUSED(fd);
    return 0;
}

int android_recorder_setVideoSize(MediaRecorderWrapper *mr, int width, int height)
{
    Q_UNUSED(mr);
    Q_UNUSED(width);
    Q_UNUSED(height);
    return 0;
}

int android_recorder_setVideoFrameRate(MediaRecorderWrapper *mr, int frames_per_second)
{
    Q_UNUSED(mr);
    Q_UNUSED(frames_per_second);
    return 0;
}

int android_recorder_setParameters(MediaRecorderWrapper *mr, const char* parameters)
{
    Q_UNUSED(mr);
    Q_UNUSED(parameters);
    return 0;
}

int android_recorder_start(MediaRecorderWrapper *mr)
{
    Q_UNUSED(mr);
    return 0;
}

int android_recorder_stop(MediaRecorderWrapper *mr)
{
    Q_UNUSED(mr);
    return 0;
}

int android_recorder_prepare(MediaRecorderWrapper *mr)
{
    Q_UNUSED(mr);
    return 0;
}

int android_recorder_reset(MediaRecorderWrapper *mr)
{
    Q_UNUSED(mr);
    return 0;
}

int android_recorder_close(MediaRecorderWrapper *mr)
{
    Q_UNUSED(mr);
    return 0;
}

int android_recorder_release(MediaRecorderWrapper *mr)
{
    Q_UNUSED(mr);
    return 0;
}
