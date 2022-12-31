/*
 * Copyright (C) 2013 Canonical, Ltd.
 * Copyright (C) 2022 UBports Foundation.
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

#ifndef SNAPSHOTGENERATOR_H
#define SNAPSHOTGENERATOR_H

#include <QImage>

#include <qopengl.h>

class CameraControl;

class SnapshotGenerator
{
public:
    SnapshotGenerator();

    QImage snapshot(GLuint textureId, const CameraControl *control);

    void setSize(int width, int height);

private:
    const char *vertexShader() const;
    const char *fragmentShader() const;
    GLuint loadShader(GLenum shaderType, const char* pSource);
    GLuint createProgram(const char* pVertexSource, const char* pFragmentSource);

    int m_width;
    int m_height;

    GLint position_loc;
    GLint v_matrix_loc;
    GLint tex_coord_loc;
    GLint sampler_loc;
    GLint tex_matrix_loc;

    QImage m_snapshotImage;
};

#endif // SNAPSHOTGENERATOR_H
