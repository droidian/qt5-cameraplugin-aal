include(../../coverage.pri)

CONFIG += testcase
CONFIG += no_private_qt_headers_warning
TARGET = tst_aalcameracontrol

QT += testlib multimedia-private opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalcameracontrol.h \
    ../../src/aalcameraservice.h

SOURCES += tst_aalcameracontrol.cpp \
    ../../src/aalcameracontrol.cpp \
    aalcameraservice.cpp
