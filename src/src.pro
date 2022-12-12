include(../coverage.pri)
TARGET = aalcamera
TEMPLATE = lib
CONFIG += plugin
QT += concurrent multimedia opengl gui sensors

PLUGIN_TYPE = mediaservice

target.path += $$[QT_INSTALL_PLUGINS]/$${PLUGIN_TYPE}
INSTALLS = target

CONFIG += link_pkgconfig
PKGCONFIG += exiv2 libqtubuntu-media-signals libmedia libcamera hybris-egl-platform libpulse-simple libandroid-properties android-headers deviceinfo
LIBS += -lEGL -lui

OTHER_FILES += aalcamera.json

HEADERS += \
    aalcameracontrol.h \
    aalcameraflashcontrol.h \
    aalcamerafocuscontrol.h \
    aalcameraservice.h \
    aalcameraserviceplugin.h \
    aalcamerazoomcontrol.h \
    aalimagecapturecontrol.h \
    aalimageencodercontrol.h \
    aalmediarecordercontrol.h \
    aalmetadatawritercontrol.h \
    aalvideodeviceselectorcontrol.h \
    aalvideoencodersettingscontrol.h \
    aalvideorenderercontrol.h \
    aalviewfindersettingscontrol.h \
    aalcamerainfocontrol.h \
    audiocapture.h \
    aalcameraexposurecontrol.h \
    storagemanager.h \
    rotationhandler.h

SOURCES += \
    aalcameracontrol.cpp \
    aalcameraflashcontrol.cpp \
    aalcamerafocuscontrol.cpp \
    aalcameraservice.cpp \
    aalcameraserviceplugin.cpp \
    aalcamerazoomcontrol.cpp \
    aalimagecapturecontrol.cpp \
    aalimageencodercontrol.cpp \
    aalmediarecordercontrol.cpp \
    aalmetadatawritercontrol.cpp \
    aalvideodeviceselectorcontrol.cpp \
    aalvideoencodersettingscontrol.cpp \
    aalvideorenderercontrol.cpp \
    aalviewfindersettingscontrol.cpp \
    aalcamerainfocontrol.cpp \
    audiocapture.cpp \
    aalcameraexposurecontrol.cpp \
    storagemanager.cpp \
    rotationhandler.cpp
