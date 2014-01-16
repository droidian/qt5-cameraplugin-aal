include(../../coverage.pri)

TARGET = tst_aalviewfindersettingscontrol

QT += testlib multimedia opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalviewfindersettingscontrol.h \
    ../../src/aalcameraservice.h \
    ../../src/aalvideorenderercontrol.h

SOURCES += tst_aalviewfindersettingscontrol.cpp \
    ../../src/aalviewfindersettingscontrol.cpp \
    aalcameraservice.cpp \
    aalvideorenderercontrol.cpp

check.depends = $${TARGET}
check.commands = ./$${TARGET}
QMAKE_EXTRA_TARGETS += check
