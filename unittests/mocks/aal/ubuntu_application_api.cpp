/*
 * Copyright (C) 2013 Canonical, Ltd.
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

#include <cstddef>
#include <stdint.h>

#include <ubuntu/application/ui/display.h>

#define UNUSED __attribute__((unused))

UAUiDisplay* ua_ui_display_new_with_index(UNUSED size_t index)
{
    UAUiDisplay* display = 0;
    return display;
}

void ua_ui_display_destroy(UNUSED UAUiDisplay* display)
{
}

uint32_t ua_ui_display_query_horizontal_res(UNUSED UAUiDisplay* display)
{
    return 1920;
}

uint32_t ua_ui_display_query_vertical_res(UNUSED UAUiDisplay* display)
{
    return 1080;
}
