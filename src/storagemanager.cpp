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
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QCoreApplication>

const QLatin1String photoBase = QLatin1String("image");
const QLatin1String videoBase = QLatin1String("video");
const QLatin1String photoExtension = QLatin1String("jpg");
const QLatin1String videoExtension = QLatin1String("mp4");
const QLatin1String dateFormat = QLatin1String("yyyyMMdd_HHmmsszzz");

StorageManager::StorageManager()
{
}

QString StorageManager::nextPhotoFileName(const QString &directoy)
{
    m_directory = directoy;
    if (m_directory.isEmpty()) {
        m_directory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/" + QCoreApplication::applicationName();
        QDir dir;
        dir.mkpath(m_directory);
    }

    return fileNameGenerator(photoBase, photoExtension);
}

QString StorageManager::nextVideoFileName(const QString &directoy)
{
    m_directory = directoy;
    if (m_directory.isEmpty()) {
        m_directory = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation) + "/" + QCoreApplication::applicationName();
        QDir dir;
        dir.mkpath(m_directory);
    }

    return fileNameGenerator(videoBase, videoExtension);
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

QString StorageManager::fileNameGenerator(const QString &base, const QString& extension)
{
    QString date = QDateTime::currentDateTime().toString(dateFormat);
    return QString("%1/%2%3.%4")
            .arg(m_directory)
            .arg(base)
            .arg(date)
            .arg(extension);
}
