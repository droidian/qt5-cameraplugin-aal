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

#include "aalmetadatawritercontrol.h"
#include "aalcameraservice.h"

AalMetaDataWriterControl::AalMetaDataWriterControl(AalCameraService *service, QObject *parent) :
    QMetaDataWriterControl(parent),
    m_service(service)
{
}

QStringList AalMetaDataWriterControl::availableMetaData() const
{
    QStringList keys;
    return keys;
}

bool AalMetaDataWriterControl::isMetaDataAvailable() const
{
    return true;
}

bool AalMetaDataWriterControl::isWritable() const
{
    return true;
}

QVariant AalMetaDataWriterControl::metaData(const QString &key) const
{
    Q_UNUSED(key);
    return QVariant();
}

void AalMetaDataWriterControl::setMetaData(const QString &key, const QVariant &value)
{
    m_metaData[key] = value;
}

int AalMetaDataWriterControl::orientation() const
{
    return m_orientation;
}
