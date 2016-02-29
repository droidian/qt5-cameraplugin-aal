include(../../coverage.pri)

TARGET = tst_aalcamerafocuscontrol

QT += testlib multimedia opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalcamerafocuscontrol.h \
    ../../src/aalcameraservice.h \
    ../../src/aalimagecapturecontrol.h \
    ../../src/storagemanager.h

SOURCES += tst_aalcamerafocuscontrol.cpp \
    ../../src/aalcamerafocuscontrol.cpp \
    storagemanager.cpp \
    aalcameraservice.cpp \
    aalimagecapturecontrol.cpp

check.depends = $${TARGET}
check.commands = ./$${TARGET}
QMAKE_EXTRA_TARGETS += check
