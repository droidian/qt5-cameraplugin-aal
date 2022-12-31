/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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

#include "media_signals.h"

#include <QMutexLocker>

SharedSignal *SharedSignal::m_sharedSignal = NULL;
QMutex SharedSignal::m_mutex;

SharedSignal::SharedSignal(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<SharedSignal::Orientation>("SharedSignal::Orientation");
}

SharedSignal* SharedSignal::instance()
{
    QMutexLocker locker(&m_mutex);
    if (m_sharedSignal == NULL)
        m_sharedSignal = new SharedSignal();

    return m_sharedSignal;
}
