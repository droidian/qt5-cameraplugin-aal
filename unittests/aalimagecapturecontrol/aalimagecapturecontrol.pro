include(../../coverage.pri)

CONFIG += testcase
CONFIG += no_private_qt_headers_warning
TARGET = tst_aalimagecapturecontrol

QT += testlib multimedia-private opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalimagecapturecontrol.h \
    ../../src/aalcameraservice.h \
    ../../src/aalimageencodercontrol.h \
    ../../src/aalvideorenderercontrol.h \
    ../../src/storagemanager.h

SOURCES += tst_aalimagecapturecontrol.cpp \
    ../../src/aalimagecapturecontrol.cpp \
    aalcameraservice.cpp \
    aalimageencodercontrol.cpp \
    aalvideorenderercontrol.cpp \
    storagemanager.cpp
