TARGET = shadervideonode
TEMPLATE = lib
CONFIG += plugin c++11 link_pkgconfig
QT += gui multimedia quick-private quick

include(../coverage.pri)

PLUGIN_TYPE = video/videonode
PLUGIN_EXTENDS = quick
PLUGIN_CLASS_NAME = ShaderVideoNodePlugin

target.path += $$[QT_INSTALL_PLUGINS]/$${PLUGIN_TYPE}
INSTALLS = target

INCLUDEPATH += /usr/include/hybris/camera /usr/include/hybris/media
LIBS += -lhybris_ics -lcamera -lmedia -lsharedsignal -L../sharedsignal/

OTHER_FILES += shadervideonode.json

HEADERS += \
    shadervideonodeplugin.h \
    shadervideomaterial.h \
    shadervideoshader.h \
    shadervideonode.h \
    snapshotgenerator.h \
    video_sink.h \
    video_sink_p.h \
    egl_video_sink.h \
    media_signals.h

SOURCES += \
    shadervideonodeplugin.cpp \
    shadervideomaterial.cpp \
    shadervideoshader.cpp \
    shadervideonode.cpp \
    snapshotgenerator.cpp \
    qsgvideonode_p.cpp \
    video_sink.cpp \
    egl_video_sink.cpp