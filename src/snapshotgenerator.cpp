/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Guenter Schwann <guenter.schwann@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "snapshotgenerator.h"

#include <QGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLShader>

#include <cstdio>

SnapshotGenerator::SnapshotGenerator()
    : m_width(0),
      m_height(0),
      position_loc(0),
      v_matrix_loc(0),
      tex_coord_loc(0),
      sampler_loc(0),
      tex_matrix_loc(0)
{
}

/**
 * @brief SnapshotGenerator::snapshot
 * @param textureId Texture to be stored as QImage
 * @return Image containing the content of the last texture
 */
QImage SnapshotGenerator::snapshot(GLuint textureId, GLfloat textureMatrix[])
{
    QGLFramebufferObject fbo(m_width, m_height);
    QPainter paint(&fbo);
    fbo.bind();

#ifdef __arm__
    QOpenGLShaderProgram program;
    program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShader());
    program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShader());

    program.link();
    program.bind();

    glViewport(0, 0, m_width, m_height);

    position_loc = program.attributeLocation("a_position");
    v_matrix_loc = program.uniformLocation("v_matrix");
    sampler_loc = program.uniformLocation("s_texture");
    tex_coord_loc = program.attributeLocation("a_texCoord");
    tex_matrix_loc = program.uniformLocation("m_texMatrix");

    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLfloat vVertices[] = {
        0.0f, 00.0f, 0.0f, // Position 0
        0.0f, 1.0f, 0.0f, // Position 1
        1.0f, 1.0f, 0.0f, // Position 2
        1.0f, 00.0f, 0.0f, // Position 3
    };
    vVertices[4] = m_height;
    vVertices[6] = m_width;
    vVertices[7] = m_height;
    vVertices[9] = m_width;

    GLfloat tVertices[] = {
        0.0f, 1.0f, // TexCoord 1
        0.0f, 0.0f, // TexCoord 0
        1.0f, 0.0f, // TexCoord 3
        1.0f, 1.0f // TexCoord 2
    };

    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    program.enableAttributeArray(position_loc);
    program.setAttributeArray(position_loc, vVertices, 3);
    QMatrix4x4 pmvMatrix;
    pmvMatrix.ortho(QRect(0,0,m_width,m_height));
    program.setUniformValue(v_matrix_loc, pmvMatrix);

    program.enableAttributeArray(tex_coord_loc);
    program.setAttributeArray(tex_coord_loc, tVertices, 2);

    program.enableAttributeArray(sampler_loc);
    program.setUniformValue(sampler_loc, 0);
    program.enableAttributeArray(tex_matrix_loc);

    QMatrix4x4 texMat(textureMatrix);
    texMat = texMat.transposed();
    program.setUniformValue(tex_matrix_loc, texMat);

    glBindTexture(GL_TEXTURE_2D, textureId);
    glActiveTexture(GL_TEXTURE0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
#endif

    return fbo.toImage();
}

void SnapshotGenerator::setSize(int width, int height)
{
    m_width = width;
    m_height = height;
}

const char *SnapshotGenerator::vertexShader() const
{
    return
        "#extension GL_OES_EGL_image_external : require              \n"
        "attribute vec4 a_position;                                  \n"
        "uniform highp mat4 v_matrix;                                \n"
        "attribute vec2 a_texCoord;                                  \n"
        "uniform mat4 m_texMatrix;                                   \n"
        "varying vec2 v_texCoord;                                    \n"
        "void main()                                                 \n"
        "{                                                           \n"
        "   gl_Position = v_matrix * a_position;                     \n"
        "   v_texCoord = (m_texMatrix * vec4(a_texCoord, 0.0, 1.0)).xy;\n"
        "}                                                           \n";
}

const char *SnapshotGenerator::fragmentShader() const
{
    return
        "#extension GL_OES_EGL_image_external : require      \n"
        "precision mediump float;                            \n"
        "varying vec2 v_texCoord;                            \n"
        "uniform samplerExternalOES s_texture;               \n"
        "void main()                                         \n"
        "{                                                   \n"
        "    gl_FragColor = texture2D( s_texture, v_texCoord );\n"
        "}                                                   \n";
}
