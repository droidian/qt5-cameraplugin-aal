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

#include "aalviewfindersettingscontrol.h"
#include "aalcameraservice.h"

AalViewfinderSettingsControl::AalViewfinderSettingsControl(AalCameraService *service, QObject *parent)
    :QCameraViewfinderSettingsControl(parent),
      m_service(service),
      m_currentSize(),
      m_currentFPS(30),
      m_minFPS(10),
      m_maxFPS(30)
{
}

AalViewfinderSettingsControl::~AalViewfinderSettingsControl()
{
}

bool AalViewfinderSettingsControl::isViewfinderParameterSupported(ViewfinderParameter parameter) const
{
    Q_UNUSED(parameter);
    return false;
}

void AalViewfinderSettingsControl::setViewfinderParameter(ViewfinderParameter parameter, const QVariant &value)
{
    Q_UNUSED(parameter);
    Q_UNUSED(value);
}

QVariant AalViewfinderSettingsControl::viewfinderParameter(ViewfinderParameter parameter) const
{
    Q_UNUSED(parameter);
    return QVariant();
}

void AalViewfinderSettingsControl::resetAllSettings()
{
}
