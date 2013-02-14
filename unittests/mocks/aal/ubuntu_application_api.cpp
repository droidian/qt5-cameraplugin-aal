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

#include "ubuntu_application_ui.h"

#include <iostream>

using namespace std;

void ubuntu_application_ui_create_display_info(__attribute__((unused))ubuntu_application_ui_physical_display_info *info, __attribute__((unused))size_t index)
{
}

void ubuntu_application_ui_destroy_display_info(__attribute__((unused))ubuntu_application_ui_physical_display_info info)
{
}

int32_t ubuntu_application_ui_query_horizontal_resolution(__attribute__((unused))ubuntu_application_ui_physical_display_info info)
{
    return 1920;
}

int32_t ubuntu_application_ui_query_vertical_resolution(__attribute__((unused))ubuntu_application_ui_physical_display_info info)
{
    return 1080;
}
