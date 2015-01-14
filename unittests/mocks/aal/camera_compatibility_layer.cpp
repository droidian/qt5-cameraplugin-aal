/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#include "camera_compatibility_layer.h"
#include "camera_compatibility_layer_capabilities.h"

#include "camera_control.h"

#include <QtGlobal>
#include <QDebug>

void crashTest(CameraControl* control)
{
    if (control->listener == 0)
        qDebug() << "Something is wrong, but at least it did not crash";
}


int android_camera_get_number_of_devices()
{
    return 2;
}

CameraControl* android_camera_connect_to(CameraType camera_type, CameraControlListener* listener)
{    
    Q_UNUSED(camera_type);
    CameraControl* cc = new CameraControl();
    cc->listener = listener;
    return cc;
}

void android_camera_disconnect(CameraControl* control)
{
    crashTest(control);
}

int android_camera_lock(CameraControl* control)
{
    Q_UNUSED(control);
    return 0;
}

int android_camera_unlock(CameraControl* control)
{
    Q_UNUSED(control);
    return 0;
}

void android_camera_delete(CameraControl* control)
{
    delete control;
}

void android_camera_dump_parameters(CameraControl* control)
{
    crashTest(control);
}

void android_camera_set_flash_mode(CameraControl* control, FlashMode mode)
{
    Q_UNUSED(mode);
    crashTest(control);
}

void android_camera_get_flash_mode(CameraControl* control, FlashMode* mode)
{
    Q_UNUSED(mode);
    crashTest(control);
}

void android_camera_set_white_balance_mode(CameraControl* control, WhiteBalanceMode mode)
{
    Q_UNUSED(mode);
    crashTest(control);
}

void android_camera_get_white_balance_mode(CameraControl* control, WhiteBalanceMode* mode)
{
    Q_UNUSED(mode);
    crashTest(control);
}

void android_camera_enumerate_supported_scene_modes(CameraControl* control, scene_mode_callback cb, void* ctx)
{
    Q_UNUSED(cb);
    Q_UNUSED(ctx);
    crashTest(control);
    cb(ctx, SCENE_MODE_ACTION);
}

void android_camera_enumerate_supported_flash_modes(CameraControl* control, flash_mode_callback cb, void* ctx)
{
    Q_UNUSED(cb);
    Q_UNUSED(ctx);
    crashTest(control);
    cb(ctx, FLASH_MODE_ON);
    cb(ctx, FLASH_MODE_AUTO);
    cb(ctx, FLASH_MODE_TORCH);
    cb(ctx, FLASH_MODE_OFF);
}

void android_camera_set_scene_mode(CameraControl* control, SceneMode mode)
{
    Q_UNUSED(mode);
    crashTest(control);
}

void android_camera_get_scene_mode(CameraControl* control, SceneMode* mode)
{
    Q_UNUSED(mode);
    crashTest(control);
}

void android_camera_set_auto_focus_mode(CameraControl* control, AutoFocusMode mode)
{
    Q_UNUSED(mode);
    crashTest(control);
}

void android_camera_get_auto_focus_mode(CameraControl* control, AutoFocusMode* mode)
{
    Q_UNUSED(mode);
    crashTest(control);
}

void android_camera_set_jpeg_quality(CameraControl* control, int quality)
{
    Q_UNUSED(quality);
    crashTest(control);
}

void android_camera_get_jpeg_quality(CameraControl* control, int* quality)
{
    Q_UNUSED(quality);
    crashTest(control);
}

void android_camera_set_effect_mode(CameraControl* control, EffectMode mode)
{
    Q_UNUSED(mode);
    crashTest(control);
}

void android_camera_get_effect_mode(CameraControl* control, EffectMode* mode)
{
    Q_UNUSED(mode);
    crashTest(control);
}

void android_camera_get_preview_fps_range(CameraControl* control, int* min, int* max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
    crashTest(control);
}

void android_camera_set_preview_fps(CameraControl* control, int fps)
{
    Q_UNUSED(fps);
    crashTest(control);
}

void android_camera_get_preview_fps(CameraControl* control, int* fps)
{
    Q_UNUSED(fps);
    crashTest(control);
}

void android_camera_enumerate_supported_preview_sizes(CameraControl* control, size_callback cb, void* ctx)
{
    Q_UNUSED(cb);
    Q_UNUSED(ctx);
    crashTest(control);
}

void android_camera_enumerate_supported_picture_sizes(CameraControl* control, size_callback cb, void* ctx)
{
    Q_UNUSED(cb);
    Q_UNUSED(ctx);
    crashTest(control);
}

void android_camera_get_preview_size(CameraControl* control, int* width, int* height)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
    crashTest(control);
}

void android_camera_set_preview_size(CameraControl* control, int width, int height)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
    crashTest(control);
}

void android_camera_get_picture_size(CameraControl* control, int* width, int* height)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
    crashTest(control);
}

void android_camera_set_picture_size(CameraControl* control, int width, int height)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
    crashTest(control);
}

void android_camera_get_current_zoom(CameraControl* control, int* zoom)
{
    Q_UNUSED(zoom);
    crashTest(control);
}

void android_camera_get_max_zoom(CameraControl* control, int* zoom)
{
    crashTest(control);
    *zoom = 4;
}

void android_camera_set_display_orientation(CameraControl* control, int32_t clockwise_rotation_degree)
{
    Q_UNUSED(clockwise_rotation_degree);
    crashTest(control);
}

void android_camera_get_preview_texture_transformation(CameraControl* control, float m[16])
{
    Q_UNUSED(m);
    crashTest(control);
}

void android_camera_update_preview_texture(CameraControl* control)
{
    crashTest(control);
}

void android_camera_set_preview_texture(CameraControl* control, int texture_id)
{
    Q_UNUSED(texture_id);
    crashTest(control);
}

void android_camera_start_preview(CameraControl* control)
{
    crashTest(control);
}

void android_camera_stop_preview(CameraControl* control)
{
    crashTest(control);
}

void android_camera_start_autofocus(CameraControl* control)
{
    crashTest(control);
}

void android_camera_stop_autofocus(CameraControl* control)
{
    crashTest(control);
}

void android_camera_start_zoom(CameraControl* control, int32_t zoom)
{
    Q_UNUSED(zoom);
    crashTest(control);
}

void android_camera_set_zoom(CameraControl* control, int32_t zoom)
{
    Q_UNUSED(zoom);
    crashTest(control);
}

void android_camera_stop_zoom(CameraControl* control)
{
    crashTest(control);
}

void android_camera_take_snapshot(CameraControl* control)
{
    crashTest(control);
}

void android_camera_set_focus_region(CameraControl* control, FocusRegion* region)
{
    Q_UNUSED(region);
    crashTest(control);
}

void android_camera_reset_focus_region(CameraControl* control)
{
    crashTest(control);
}

void android_camera_set_rotation(CameraControl* control, int rotation)
{
    Q_UNUSED(rotation);
    crashTest(control);
}

void android_camera_set_location(CameraControl* control, const float* latitude, const float* longitude, const float* altitude, int timestamp, const char* method)
{
    Q_UNUSED(latitude);
    crashTest(control);
}
