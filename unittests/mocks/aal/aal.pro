include(../../../coverage.pri)
TEMPLATE = lib
TARGET = aal
CONFIG += staticlib

HEADERS =  camera_compatibility_layer.h \
           camera_compatibility_layer_capabilities.h \
           camera_control.h \
           media_recorder_layer.h

SOURCES += camera_compatibility_layer.cpp \
           media_recorder_layer.cpp
