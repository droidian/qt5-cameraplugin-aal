include(../../coverage.pri)

TARGET = tst_aalmediarecordercontrol

QT += testlib multimedia opengl sensors

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalmediarecordercontrol.h \
    ../../src/aalcameraservice.h \
    ../../src/aalvideoencodersettingscontrol.h \
    ../../src/aalmetadatawritercontrol.h \
    ../../src/audiocapture.h \
    ../../src/storagemanager.h \
    ../../src/rotationhandler.h

SOURCES += tst_aalmediarecordercontrol.cpp \
    ../stubs/audiocapture_stub.cpp \
    ../../src/aalmediarecordercontrol.cpp \
    ../stubs/aalcameraservice_stub.cpp \
    ../stubs/aalvideoencodersettingscontrol_stub.cpp \
    ../stubs/aalmetadatawritercontrol_stub.cpp \
    ../stubs/storagemanager_stub.cpp \
    ../stubs/rotationhandler_stub.cpp

check.depends = $${TARGET}
check.commands = ./$${TARGET}
QMAKE_EXTRA_TARGETS += check
