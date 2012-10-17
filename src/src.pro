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
    aalcameraservice.h \
    aalcameraserviceplugin.h \
    aalimagecapturecontrol.h \
    aalvideorenderercontrol.h \
    storagemanager.h

SOURCES += \
    aalcameracontrol.cpp \
    aalcameraservice.cpp \
    aalcameraserviceplugin.cpp \
    aalimagecapturecontrol.cpp \
    aalvideorenderercontrol.cpp \
    storagemanager.cpp
