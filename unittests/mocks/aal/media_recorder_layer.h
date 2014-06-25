/*
 * Copyright (C) 2013 Canonical Ltd
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
 */

#ifndef RECORDER_COMPATIBILITY_LAYER_H_
#define RECORDER_COMPATIBILITY_LAYER_H_

#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

    struct MediaRecorderWrapper;
    struct CameraControl;

    // values are from andoid /frameworks/av/include/media/mediarecorder.h
    typedef enum
    {
        ANDROID_VIDEO_SOURCE_DEFAULT = 0,
        ANDROID_VIDEO_SOURCE_CAMERA = 1,
        ANDROID_VIDEO_SOURCE_GRALLOC_BUFFER = 2
    } VideoSource;

    // values are from andoid /system/core/include/system/audio.h
    typedef enum
    {
        ANDROID_AUDIO_SOURCE_DEFAULT             = 0,
        ANDROID_AUDIO_SOURCE_MIC                 = 1,
        ANDROID_AUDIO_SOURCE_VOICE_UPLINK        = 2,
        ANDROID_AUDIO_SOURCE_VOICE_DOWNLINK      = 3,
        ANDROID_AUDIO_SOURCE_VOICE_CALL          = 4,
        ANDROID_AUDIO_SOURCE_CAMCORDER           = 5,
        ANDROID_AUDIO_SOURCE_VOICE_RECOGNITION   = 6,
        ANDROID_AUDIO_SOURCE_VOICE_COMMUNICATION = 7,
        ANDROID_AUDIO_SOURCE_REMOTE_SUBMIX       = 8,
        ANDROID_AUDIO_SOURCE_CNT,
        ANDROID_AUDIO_SOURCE_MAX                 = ANDROID_AUDIO_SOURCE_CNT - 1
    } AudioSource;

    // values are from andoid /frameworks/av/include/media/mediarecorder.h
    typedef enum
    {
        ANDROID_OUTPUT_FORMAT_DEFAULT = 0,
        ANDROID_OUTPUT_FORMAT_THREE_GPP = 1,
        ANDROID_OUTPUT_FORMAT_MPEG_4 = 2,
        ANDROID_OUTPUT_FORMAT_AUDIO_ONLY_START = 3,
        /* These are audio only file formats */
        ANDROID_OUTPUT_FORMAT_RAW_AMR = 3, //to be backward compatible
        ANDROID_OUTPUT_FORMAT_AMR_NB = 3,
        ANDROID_OUTPUT_FORMAT_AMR_WB = 4,
        ANDROID_OUTPUT_FORMAT_AAC_ADIF = 5,
        ANDROID_OUTPUT_FORMAT_AAC_ADTS = 6,
        /* Stream over a socket, limited to a single stream */
        ANDROID_OUTPUT_FORMAT_RTP_AVP = 7,
        /* H.264/AAC data encapsulated in MPEG2/TS */
        ANDROID_OUTPUT_FORMAT_MPEG2TS = 8
    } OutputFormat;

    // values are from andoid /frameworks/av/include/media/mediarecorder.h
    typedef enum
    {
        ANDROID_VIDEO_ENCODER_DEFAULT = 0,
        ANDROID_VIDEO_ENCODER_H263 = 1,
        ANDROID_VIDEO_ENCODER_H264 = 2,
        ANDROID_VIDEO_ENCODER_MPEG_4_SP = 3
    } VideoEncoder;

    // values are from andoid /frameworks/av/include/media/mediarecorder.h
    typedef enum
    {
        ANDROID_AUDIO_ENCODER_DEFAULT = 0,
        ANDROID_AUDIO_ENCODER_AMR_NB = 1,
        ANDROID_AUDIO_ENCODER_AMR_WB = 2,
        ANDROID_AUDIO_ENCODER_AAC = 3,
        ANDROID_AUDIO_ENCODER_HE_AAC = 4,
        ANDROID_AUDIO_ENCODER_AAC_ELD = 5
    } AudioEncoder;

    // Callback types
    typedef void (*on_recorder_msg_error)(void *context);

    // Callback setters
    void android_recorder_set_error_cb(MediaRecorderWrapper *mr, on_recorder_msg_error cb,
                                       void *context);

    // Main recorder control API
    MediaRecorderWrapper *android_media_new_recorder();
    int android_recorder_initCheck(MediaRecorderWrapper *mr);
    int android_recorder_setCamera(MediaRecorderWrapper *mr, CameraControl* control);
    int android_recorder_setVideoSource(MediaRecorderWrapper *mr, VideoSource vs);
    int android_recorder_setAudioSource(MediaRecorderWrapper *mr, AudioSource as);
    int android_recorder_setOutputFormat(MediaRecorderWrapper *mr, OutputFormat of);
    int android_recorder_setVideoEncoder(MediaRecorderWrapper *mr, VideoEncoder ve);
    int android_recorder_setAudioEncoder(MediaRecorderWrapper *mr, AudioEncoder ae);
    int android_recorder_setOutputFile(MediaRecorderWrapper *mr, int fd);
    int android_recorder_setVideoSize(MediaRecorderWrapper *mr, int width, int height);
    int android_recorder_setVideoFrameRate(MediaRecorderWrapper *mr, int frames_per_second);
    int android_recorder_setParameters(MediaRecorderWrapper *mr, const char* parameters);
    int android_recorder_start(MediaRecorderWrapper *mr);
    int android_recorder_stop(MediaRecorderWrapper *mr);
    int android_recorder_prepare(MediaRecorderWrapper *mr);
    int android_recorder_reset(MediaRecorderWrapper *mr);
    int android_recorder_close(MediaRecorderWrapper *mr);
    int android_recorder_release(MediaRecorderWrapper *mr);

#ifdef __cplusplus
}
#endif

#endif
