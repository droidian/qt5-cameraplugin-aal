include(../coverage.pri)
TARGET = aalcamera
QT += multimedia-private opengl

PLUGIN_TYPE = mediaservice

load(qt_plugin)
target.path += $$[QT_INSTALL_PLUGINS]/$${PLUGIN_TYPE}
INSTALLS = target

INCLUDEPATH += /usr/include/hybris/camera
LIBS += -L/usr/lib/arm-linux-gnueabi -lcamera

OTHER_FILES += aalcamera.json

HEADERS += \
    aalcameracontrol.h \
    aalcameraflashcontrol.h \
    aalcamerafocuscontrol.h \
    aalcameraservice.h \
    aalcameraserviceplugin.h \
    aalcamerazoomcontrol.h \
    aalimagecapturecontrol.h \
    aalvideodeviceselectorcontrol.h \
    aalvideorenderercontrol.h \
    aalviewfindersettingscontrol.h \
    snapshotgenerator.h \
    storagemanager.h

SOURCES += \
    aalcameracontrol.cpp \
    aalcameraflashcontrol.cpp \
    aalcamerafocuscontrol.cpp \
    aalcameraservice.cpp \
    aalcameraserviceplugin.cpp \
    aalcamerazoomcontrol.cpp \
    aalimagecapturecontrol.cpp \
    aalvideodeviceselectorcontrol.cpp \
    aalvideorenderercontrol.cpp \
    aalviewfindersettingscontrol.cpp \
    snapshotgenerator.cpp \
    storagemanager.cpp
