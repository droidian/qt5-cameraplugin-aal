include(../../coverage.pri)

CONFIG += testcase
CONFIG += no_private_qt_headers_warning
TARGET = tst_aalcameraflashcontrol

QT += testlib multimedia-private opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalcameraflashcontrol.h \
    ../../src/aalcameraservice.h

SOURCES += tst_aalcameraflashcontrol.cpp \
    ../../src/aalcameraflashcontrol.cpp \
    aalcameraservice.cpp
