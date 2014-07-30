include(../../coverage.pri)

TARGET = tst_aalcameraexposurecontrol

QT += testlib multimedia opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalcameraexposurecontrol.h \
    ../../src/aalcameraservice.h

SOURCES += tst_aalcameraexposurecontrol.cpp \
    ../../src/aalcameraexposurecontrol.cpp \
    aalcameraservice.cpp

check.depends = $${TARGET}
check.commands = ./$${TARGET}
QMAKE_EXTRA_TARGETS += check
