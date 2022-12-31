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

#include <QTransform>

#include <QtCore/qdebug.h>

#include "shadervideomaterial.h"
#include "shadervideoshader.h"
#include "video_sink.h"

#include <hybris/camera/camera_compatibility_layer.h>
#include <hybris/media/surface_texture_client_hybris.h>

ShaderVideoShader *ShaderVideoMaterial::m_videoShader = 0;

ShaderVideoMaterial::ShaderVideoMaterial(const QVideoSurfaceFormat &format)
    : m_format(format),
      m_camControl(0),
      m_textureId(0),
      m_surfaceTextureClient(0),
      m_videoSink(nullptr),
      m_readyToRender(false),
      m_orientation(SharedSignal::Orientation::rotate0)
{
    /* FIXME: workaround incorrect z-ordering when VideoOutput is rotated.
     * In the following case VideoOutput is rendered on top of all other
     * Items in the scene but one. The issue is most likely related with
     * how QtQuick scenegraph's batch renderer draws the nodes.
     * When angle is 0, the ShaderVideoNode is marked as merged and there is
     * no issue. When set to a different value it is marked forever as unmerged
     * and the z-ordering becomes incorrect. Setting the CustomCompileStep flag
     * makes it marked as unmerged from the beginning and the z-ordering issue
     * never occurs.
     *
     * VideoOutput {
     *     transform: [
     *         Rotation {
     *             axis.x: 0; axis.y: 1; axis.z: 0
     *             angle: -20
     *         }
     *     ]
     *  }
     *
     * Ref.: https://bugs.launchpad.net/camera-app/+bug/1373607
     */
    setFlag(CustomCompileStep, true);
    connect(SharedSignal::instance(), &SharedSignal::setOrientation,
            this, &ShaderVideoMaterial::onSetOrientation);
    connect(SharedSignal::instance(), &SharedSignal::sinkReset,
            this, &ShaderVideoMaterial::onSinkReset);
}

QSGMaterialShader *ShaderVideoMaterial::createShader() const
{
    if (!m_videoShader) {
        m_videoShader = new ShaderVideoShader(m_format.pixelFormat());
    }
    return m_videoShader;
}

QSGMaterialType *ShaderVideoMaterial::type() const
{
    static QSGMaterialType theType;
    return &theType;
}

void ShaderVideoMaterial::setCamControl(CameraControl *cc)
{
    if (m_camControl != cc) {
        m_camControl = cc;
    }
}

CameraControl *ShaderVideoMaterial::cameraControl() const
{
    return m_camControl;
}

void ShaderVideoMaterial::setTextureId(GLuint textureId)
{
    m_textureId = textureId;
}

void ShaderVideoMaterial::setSurfaceTextureClient(SurfaceTextureClientHybris surface_texture_client)
{
    m_surfaceTextureClient = surface_texture_client;
}

void ShaderVideoMaterial::setGLVideoSink(VideoSink &sink)
{
    m_videoSink = &sink;
}

VideoSink &ShaderVideoMaterial::glVideoSink() const
{
    return *m_videoSink;
}

void ShaderVideoMaterial::updateTexture()
{
    if (!m_camControl && !m_textureId && !m_videoSink) {
        return;
    }

    if (m_camControl != NULL) {
        android_camera_update_preview_texture(m_camControl);
        android_camera_get_preview_texture_transformation(m_camControl, textureGLMatrix());
    } else if (m_videoSink && !m_readyToRender) {
        m_readyToRender = true;
        return;
    } else if (m_videoSink && m_readyToRender) {
        if (m_videoSink->swapBuffers()) {
            m_textureMatrix = m_videoSink->transformationMatrix();
        }
    }

    // See if the video needs rotation
    if (m_orientation == SharedSignal::Orientation::rotate90 ||
        m_orientation == SharedSignal::Orientation::rotate180 ||
        m_orientation == SharedSignal::Orientation::rotate270)
    {
        m_textureMatrix = rotateAndFlip(m_textureMatrix, m_orientation);
    }
    else
    {
        undoAndroidYFlip(m_textureMatrix);
    }
}

void ShaderVideoMaterial::onSetOrientation(const SharedSignal::Orientation& orientation,
        const QSize &size)
{
    m_orientation = orientation;
    m_frameSize = size;
}

// Makes sure that when a playing a video, if a new video is requested for playback during
// playback of the first video using the same player session, that we don't try and call
// m_videoSink->swap_buffers() until a new valid m_videoSink pointer is set.
void ShaderVideoMaterial::onSinkReset()
{
    qDebug() << Q_FUNC_INFO;

    // Make sure we free any locked graphics buffer
    if (m_videoSink && m_readyToRender)
        m_videoSink->swapBuffers();

    m_videoSink = nullptr;
    m_readyToRender = false;
}

// Takes a texture matrix and desired orientation, and outputs a rotated and
// horizontally flipped matrix
QMatrix4x4 ShaderVideoMaterial::rotateAndFlip(const QMatrix4x4 &m, const SharedSignal::Orientation &orientation)
{
    QMatrix4x4 ret;

    QMatrix4x4 qRowMajorTextureMatrix(m);
    const QMatrix4x4 qFlipH (-1,  0, 0, 1,
                              0,  1, 0, 0,
                              0,  0, 1, 0,
                              0,  0, 0, 1);
    const QMatrix4x4 qFlipV ( 1,  0, 0, 0,
                              0, -1, 0, 1,
                              0,  0, 1, 0,
                              0,  0, 0, 1);

    switch (orientation)
    {
        case SharedSignal::Orientation::rotate90:
        {
            // FIXME: This matrix comes from the file container, but could not
            // get this actual matrix up from GStreamer to here, so hardcoded
            // for now.
            const QMatrix4x4 qRotate90 (0, 1, 0, 0,
                                       -1, 0, 0, 0,
                                        0, 0, 1, 0,
                                        0, 0, 0, 1);

            ret = qRowMajorTextureMatrix * qRotate90;
            // This must be done manually since OpenGLES 2.0 does not support doing the transpose
            // when uploading the matrix to the GPU. OpenGLES 3.0 supports this.
            ret = ret.transposed();
            // Undo the Android mirroring. Since we already flipped height/width and rotated,
            // do this about the vertical axis.
            ret = ret * qFlipH;
        }
        break;
        case SharedSignal::Orientation::rotate180:
        {
            ret = qRowMajorTextureMatrix * qFlipH;
        }
        break;
        case SharedSignal::Orientation::rotate270:
        {
            // FIXME: This matrix comes from the file container, but could not
            // get this actual matrix up from GStreamer to here, so hardcoded
            // for now.
            const QMatrix4x4 qRotate270( 0, 1, 0, 0,
                                        -1, 0, 0, 0,
                                         0, 0, 1, 0,
                                         0, 0, 0, 1);

            ret = qRowMajorTextureMatrix * qRotate270;
            // This must be done manually since OpenGLES 2.0 does not support doing the transpose
            // when uploading the matrix to the GPU. OpenGLES 3.0 supports this.
            ret = ret.transposed();
            // Undo the Android mirroring. Since we already flipped height/width and rotated,
            // do this about the horizontal axis.
            ret = ret * qFlipV;
        }
        break;
        case SharedSignal::Orientation::rotate0:
            // No-op, no rotation needed
        default:
            qDebug() << "Not rotating";
            break;
    }

    return ret;
}

void ShaderVideoMaterial::undoAndroidYFlip(QMatrix4x4 &m)
{
    float *matrix = m.data();
    // The android matrix flips the y coordinate
    // The android matrix has it's texture coordinates not from 0..1 but in between there
    // The higher value is stored in m[13], the lower one is the higher one minus the height
    GLfloat height = -matrix[5]; // invert because of the flipping
    GLfloat offset = matrix[13] - height;
    matrix[5] = height;
    matrix[13] = offset;
}

/*!
 * \brief ShaderVideoMaterial::printGLMaxtrix
 * Prints an EGL matrix (GLfloat m[16]) to stdout.
 * This function stays here for convenience in case some more debugging is necessary for the android
 * transformation matrix.
 * \param matrix Matrix to be printed to std out
 */
void ShaderVideoMaterial::printGLMaxtrix(GLfloat matrix[])
{
    qDebug() << matrix[0] << matrix[4] << matrix[8] << matrix[12];
    qDebug() << matrix[1] << matrix[5] << matrix[9] << matrix[13];
    qDebug() << matrix[2] << matrix[6] << matrix[10] << matrix[14];
    qDebug() << matrix[3] << matrix[7] << matrix[11] << matrix[15];
}

/*!
 * \brief Prints a row-major matrix to stdout.
 * \param matrix Matrix to be printed to std out
 */
void ShaderVideoMaterial::printMaxtrix(float matrix[])
{
    qDebug() << matrix[0] << matrix[1] << matrix[2] << matrix[3];
    qDebug() << matrix[4] << matrix[5] << matrix[6] << matrix[7];
    qDebug() << matrix[8] << matrix[9] << matrix[10] << matrix[11];
    qDebug() << matrix[12] << matrix[13] << matrix[14] << matrix[15];
}
