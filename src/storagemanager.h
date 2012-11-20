/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Guenter Schwann <guenter.schwann@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QString>

class StorageManager
{
public:
    StorageManager();

    QString nextPhotoFileName(const QString &directoy = QString());
    QString nextVideoFileName(const QString &directoy = QString());

    bool checkDirectory(const QString &path) const;

private:
    QString nextMediaFileName(const QString &extension);
    QString fileNameGenerator(int idx, const QString &extension);

    QString m_directory;
};

#endif // STORAGEMANAGER_H
