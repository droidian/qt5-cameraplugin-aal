include(../../coverage.pri)

TARGET = tst_aalcamerazoomcontrol

QT += testlib multimedia opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalcamerazoomcontrol.h \
    ../../src/aalcameracontrol.h \
    ../../src/aalcameraservice.h

SOURCES += tst_aalcamerazoomcontrol.cpp \
    ../../src/aalcamerazoomcontrol.cpp \
    ../stubs/aalcameracontrol_stub.cpp \
    aalcameraservice.cpp

check.depends = $${TARGET}
check.commands = ./$${TARGET}
QMAKE_EXTRA_TARGETS += check
