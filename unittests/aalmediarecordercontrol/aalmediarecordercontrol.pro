include(../../coverage.pri)

CONFIG += testcase
CONFIG += no_private_qt_headers_warning
TARGET = tst_aalmediarecordercontrol

QT += testlib multimedia-private opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalmediarecordercontrol.h \
    ../../src/aalcameraservice.h \
    ../../src/aalmetadatawritercontrol.h \
    ../../src/storagemanager.h

SOURCES += tst_aalmediarecordercontrol.cpp \
    ../../src/aalmediarecordercontrol.cpp \
    aalcameraservice.cpp \
    ../stubs/aalmetadatawritercontrol_stub.cpp \
    storagemanager.cpp
