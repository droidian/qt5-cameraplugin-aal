include(../../coverage.pri)

CONFIG += no_private_qt_headers_warning
TARGET = tst_aalcamerafocuscontrol

QT += testlib multimedia-private opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalcamerafocuscontrol.h \
    ../../src/aalcameraservice.h \
    ../../src/aalimagecapturecontrol.h

SOURCES += tst_aalcamerafocuscontrol.cpp \
    ../../src/aalcamerafocuscontrol.cpp \
    aalcameraservice.cpp \
    aalimagecapturecontrol.cpp

check.depends = $${TARGET}
check.commands = ./$${TARGET}
QMAKE_EXTRA_TARGETS += check
