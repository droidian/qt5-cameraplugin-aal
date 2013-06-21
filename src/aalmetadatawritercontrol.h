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

#ifndef AALMETADATAWRITERCONTROL_H
#define AALMETADATAWRITERCONTROL_H

#include <QMap>
#include <QMetaDataWriterControl>
#include <QVariant>

class AalCameraService;

/*!
 * \brief The AalMetaDataWriterControl class handles the writable metadata
 * Implementation for QMetaDataWriterControl
 * http://qt-project.org/doc/qt-5.0/qtmultimedia/qmetadatawritercontrol.html
 * A list of already defined tags is listed here
 * http://qt-project.org/doc/qt-5.0/qtmultimedia/qmediametadata.html
 */
class AalMetaDataWriterControl : public QMetaDataWriterControl
{
    Q_OBJECT
public:
    explicit AalMetaDataWriterControl(AalCameraService *service, QObject *parent = 0);
    
    QStringList availableMetaData() const;
    bool isMetaDataAvailable() const;
    bool isWritable() const;
    QVariant metaData(const QString & key) const;
    void setMetaData(const QString & key, const QVariant & value);

    int orientation() const;
    int correctedOrientation() const;

    void clearAllMetaData();

private:
    AalCameraService *m_service;
    QMap<QString, QVariant> m_metaData;
    int m_orientation;
};

#endif // AALMETADATAWRITERCONTROL_H
