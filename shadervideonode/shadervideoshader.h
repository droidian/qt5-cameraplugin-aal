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

#ifndef SHADERVIDEOSHADER_H
#define SHADERVIDEOSHADER_H

#include <qglobal.h>
#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0))
#include <QSGMaterialShader>
#else
#include <QtQuick/QSGMaterialShader>
#endif
#include <QVideoFrame>

class ShaderVideoShader : public QSGMaterialShader
{
public:
    ShaderVideoShader(QVideoFrame::PixelFormat pixelFormat);

    void updateState(const RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial);

    char const *const *attributeNames() const;

    int m_tex_matrix;

protected:
    const char *vertexShader() const;
    const char *fragmentShader() const;

    void initialize();

    int m_id_matrix;
    int m_id_texture;
    int m_id_opacity;
    QVideoFrame::PixelFormat m_pixelFormat;
};

#endif // SHADERVIDEOSHADER_H
