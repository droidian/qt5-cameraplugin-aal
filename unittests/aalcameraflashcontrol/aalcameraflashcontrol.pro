include(../../coverage.pri)

TARGET = tst_aalcameraflashcontrol

QT += testlib multimedia opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalcameraflashcontrol.h \
    ../../src/aalcameracontrol.h \
    ../../src/aalcameraservice.h

SOURCES += tst_aalcameraflashcontrol.cpp \
    ../../src/aalcameraflashcontrol.cpp \
    ../stubs/aalcameracontrol_stub.cpp \
    aalcameraservice.cpp

check.depends = $${TARGET}
check.commands = ./$${TARGET}
QMAKE_EXTRA_TARGETS += check
