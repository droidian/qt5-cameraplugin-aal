/*
 * Copyright (C) 2016 Canonical, Ltd.
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

StorageManager::StorageManager(QObject* parent) : QObject(parent)
{
}

QString StorageManager::nextPhotoFileName(const QString &directoy)
{
    Q_UNUSED(directoy);
    return QString();
}

QString StorageManager::nextVideoFileName(const QString &directoy)
{
    Q_UNUSED(directoy);
    return QString();
}

bool StorageManager::checkDirectory(const QString &path) const
{
    Q_UNUSED(path);
    return true;
}

QString StorageManager::fileNameGenerator(const QString &base, const QString& extension)
{
    Q_UNUSED(base);
    Q_UNUSED(extension);
    return QString();
}

bool StorageManager::updateJpegMetadata(QByteArray data, QVariantMap metadata, QTemporaryFile* destination)
{
    Q_UNUSED(data);
    Q_UNUSED(metadata);
    Q_UNUSED(destination);

    return true;
}

SaveToDiskResult StorageManager::saveJpegImage(QByteArray data, QVariantMap metadata,
                                               QString fileName, QSize previewResolution, int captureID)
{
    Q_UNUSED(data);
    Q_UNUSED(metadata);
    Q_UNUSED(fileName);
    Q_UNUSED(captureID);
    Q_UNUSED(previewResolution);
    return SaveToDiskResult();
}

QString StorageManager::decimalToExifRational(double decimal)
{
    Q_UNUSED(decimal);
    return QString();
}

SaveToDiskResult::SaveToDiskResult() : success(false)
{
}
