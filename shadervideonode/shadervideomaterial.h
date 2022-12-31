/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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

#ifndef SHADERVIDEOMATERIAL_H
#define SHADERVIDEOMATERIAL_H

#include <qglobal.h>
#if (QT_VERSION < QT_VERSION_CHECK(5, 2, 0))
#include <QSGMaterial>
#else
#include <QtQuick/QSGMaterial>
#endif
#include <qopengl.h>
#include <QMatrix4x4>
#include <QObject>
#include <QPointer>
#include <QVideoSurfaceFormat>
#include "media_signals.h"
#include "video_sink.h"

#include <memory>

struct CameraControl;
struct MediaPlayerWrapper;
class ShaderVideoShader;

class ShaderVideoMaterial : public QObject, public QSGMaterial
{
    Q_OBJECT
public:
    typedef void* SurfaceTextureClientHybris;
    typedef void* GLConsumerWrapperHybris;

    ShaderVideoMaterial(const QVideoSurfaceFormat &format);

    QSGMaterialShader *createShader() const;

    virtual QSGMaterialType *type() const;

    void setCamControl(CameraControl *cc);
    CameraControl *cameraControl() const;

    void setTextureId(GLuint textureId);
    GLuint textureId() const { return m_textureId; }

    void setSurfaceTextureClient(SurfaceTextureClientHybris surface_texture_client);
    void setGLVideoSink(VideoSink &sink);
    VideoSink &glVideoSink() const;

    void updateTexture();

    GLfloat *textureGLMatrix() {
        return static_cast<GLfloat *>(m_textureMatrix.data());
    }

private Q_SLOTS:
    void onSetOrientation(const SharedSignal::Orientation& orientation, const QSize &size);
    void onSinkReset();

private:
    QMatrix4x4 rotateAndFlip(const QMatrix4x4 &m,
                             const SharedSignal::Orientation &orientation);
    void undoAndroidYFlip(QMatrix4x4 &matrix);
    void printGLMaxtrix(GLfloat matrix[]);
    void printMaxtrix(float matrix[]);

    QVideoSurfaceFormat m_format;
    CameraControl *m_camControl;
    GLuint m_textureId;
    QMatrix4x4 m_textureMatrix;
    SurfaceTextureClientHybris m_surfaceTextureClient;
    QPointer<VideoSink> m_videoSink;
    bool m_readyToRender;
    static ShaderVideoShader *m_videoShader; // the shader is cached in the Qt scene graph
    SharedSignal::Orientation m_orientation;
    QSize m_frameSize;
};

#endif // SHADERVIDEOMATERIAL_H
