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

#include "storagemanager.h"
#include <QDate>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>

const QLatin1String photoBase = QLatin1String("image");
const QLatin1String videoBase = QLatin1String("video");
const QLatin1String photoExtension = QLatin1String("jpg");
const QLatin1String videoExtension = QLatin1String("mp4");
const QLatin1String dateFormat = QLatin1String("yyyyMMdd");

StorageManager::StorageManager()
{
}

QString StorageManager::nextPhotoFileName(const QString &directoy)
{
    m_directory = directoy;
    if (m_directory.isEmpty())
        m_directory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/" + QCoreApplication::applicationName();

    return nextMediaFileName(photoBase, photoExtension);
}

QString StorageManager::nextVideoFileName(const QString &directoy)
{
    m_directory = directoy;
    if (m_directory.isEmpty())
        m_directory = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/" + QCoreApplication::applicationName();

    return nextMediaFileName(videoBase, videoExtension);
}

bool StorageManager::checkDirectory(const QString &path) const
{
    QFileInfo fi(path);
    QDir dir;
    if (fi.isDir())
        dir.setPath(path);
    else
        dir.setPath(fi.absoluteDir().absolutePath());

    if (!dir.exists()) {
        bool ok = dir.mkpath(dir.absolutePath());
        if (!ok)
            return false;
    }

    fi.setFile(dir.absolutePath());
    if (!fi.isWritable())
        return false;

    return true;
}

QString StorageManager::nextMediaFileName(const QString &base, const QString &extension)
{
    int idx = 1;
    QString fileName = fileNameGenerator(idx, base, extension);
    while (QFile::exists(fileName)) {
        ++idx;
        fileName = fileNameGenerator(idx, base, extension);
    }
    return fileName;
}

QString StorageManager::fileNameGenerator(int idx, const QString &base,
                                          const QString& extension)
{
    QString date = QDate::currentDate().toString(dateFormat);
    return QString("%1/%2%3_%4.%5")
            .arg(m_directory)
            .arg(base)
            .arg(date)
            .arg(idx,4,10,QLatin1Char('0'))
            .arg(extension);
}
