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

#ifndef SHADERVIDEONODEPLUGIN_H
#define SHADERVIDEONODEPLUGIN_H

#include <qsgvideonode_p.h>
#include <QVideoSurfaceFormat>

class ShaderVideoNodePlugin : public QObject, public QSGVideoNodeFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QSGVideoNodeFactoryInterface)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.sgvideonodefactory/5.2" FILE "shadervideonode.json")

public:
    ~ShaderVideoNodePlugin();
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;
    QSGVideoNode *createNode(const QVideoSurfaceFormat &format);
};

#endif // SHADERVIDEONODEPLUGIN_H
