include(../../coverage.pri)

CONFIG += testcase
CONFIG += no_private_qt_headers_warning
TARGET = tst_aalvideodeviceselectorcontrol

QT += testlib multimedia-private opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalvideodeviceselectorcontrol.h \
    ../../src/aalcameraservice.h

SOURCES += tst_aalvideodeviceselectorcontrol.cpp \
    ../../src/aalvideodeviceselectorcontrol.cpp \
    aalcameraservice.cpp
