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

#include <QDebug>
#include <QString>

#include "storagemanager.h"

StorageManager::StorageManager()
{
}

QString StorageManager::nextPhotoFileName(const QString &directory)
{
    Q_UNUSED(directory);
    return QString();
}

QString StorageManager::nextVideoFileName(const QString &directory)
{
    Q_UNUSED(directory);
    return QString();
}

bool StorageManager::checkDirectory(const QString &path) const
{
    Q_UNUSED(path);
    return true;
}

QString StorageManager::fileNameGenerator(const QString &base,
                                          const QString& extension)
{
    Q_UNUSED(base);
    Q_UNUSED(extension);
    return QString();
}
