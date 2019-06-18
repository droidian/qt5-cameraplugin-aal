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

#include "aalmetadatawritercontrol.h"
#include "aalcameraservice.h"

/*!
 * \brief AalMetaDataWriterControl::AalMetaDataWriterControl
 * \param service
 * \param parent
 */
AalMetaDataWriterControl::AalMetaDataWriterControl(AalCameraService *service, QObject *parent) :
    QMetaDataWriterControl(parent),
    m_service(service)
{
    Q_ASSERT(service);
}

/*!
 * \brief AalMetaDataWriterControl::availableMetaData \reimp
 * \return
 */
QStringList AalMetaDataWriterControl::availableMetaData() const
{
    QStringList keys;
    keys.reserve(m_metaData.size());
    QMap<QString, QVariant>::const_iterator i = m_metaData.constBegin();
    while (i != m_metaData.constEnd()) {
        keys.append(i.key());
        ++i;
    }
    return keys;
}

/*!
 * \brief AalMetaDataWriterControl::isMetaDataAvailable \reimp
 * \return
 */
bool AalMetaDataWriterControl::isMetaDataAvailable() const
{
    return !m_metaData.isEmpty();
}

/*!
 * \brief AalMetaDataWriterControl::isWritable \reimp
 * \return
 */
bool AalMetaDataWriterControl::isWritable() const
{
    return true;
}

/*!
 * \brief AalMetaDataWriterControl::metaData \reimp
 * \param key
 * \return
 */
QVariant AalMetaDataWriterControl::metaData(const QString &key) const
{
    if (m_metaData.contains(key))
        return m_metaData[key];
    return QVariant();
}

/*!
 * \brief AalMetaDataWriterControl::setMetaData \reimp
 * \param key
 * \param value
 */
void AalMetaDataWriterControl::setMetaData(const QString &key, const QVariant &value)
{
    m_metaData[key] = value;
}

/*!
 * \brief AalMetaDataWriterControl::clearMetaData removes all metadata
 */
void AalMetaDataWriterControl::clearAllMetaData()
{
    m_metaData.clear();
}
