include(../../../coverage.pri)
TEMPLATE = lib
TARGET = aal
CONFIG += staticlib

HEADERS =  camera_compatibility_layer.h \
           camera_compatibility_layer_capabilities.h \
           camera_control.h \
           recorder_compatibility_layer.h

SOURCES += camera_compatibility_layer.cpp \
           recorder_compatibility_layer.cpp \
           ubuntu_application_api.cpp
