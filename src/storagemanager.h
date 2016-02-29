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

#ifndef STORAGEMANAGER_H
#define STORAGEMANAGER_H

#include <QString>
#include <QVariantMap>
#include <QByteArray>
#include <QTemporaryFile>
#include <QImage>

class SaveToDiskResult
{
public:
    SaveToDiskResult();
    bool success;
    QString fileName;
    QString errorMessage;
};

class StorageManager : public QObject
{
    Q_OBJECT

public:
    explicit StorageManager(QObject* parent = 0);

    QString nextPhotoFileName(const QString &directoy = QString());
    QString nextVideoFileName(const QString &directoy = QString());

    bool checkDirectory(const QString &path) const;

    SaveToDiskResult saveJpegImage(QByteArray data, QVariantMap metadata,
                                   QString fileName, QSize previewResolution,
                                   int captureID);

Q_SIGNALS:
    void previewReady(int captureID, QImage image);

private:
    QString fileNameGenerator(const QString &base, const QString &extension);
    bool updateJpegMetadata(QByteArray data, QVariantMap metadata, QTemporaryFile* destination);
    QString decimalToExifRational(double decimal);

    QString m_directory;
};

#endif // STORAGEMANAGER_H
