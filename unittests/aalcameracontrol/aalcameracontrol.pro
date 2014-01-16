include(../../coverage.pri)

TARGET = tst_aalcameracontrol

QT += testlib multimedia opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalcameracontrol.h \
    ../../src/aalcameraservice.h

SOURCES += tst_aalcameracontrol.cpp \
    ../../src/aalcameracontrol.cpp \
    aalcameraservice.cpp

check.depends = $${TARGET}
check.commands = ./$${TARGET}
QMAKE_EXTRA_TARGETS += check
