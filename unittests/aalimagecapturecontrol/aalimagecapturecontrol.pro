include(../../coverage.pri)

TARGET = tst_aalimagecapturecontrol

QT += testlib multimedia opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalimagecapturecontrol.h \
    ../../src/aalcameraservice.h \
    ../../src/aalimageencodercontrol.h \
    ../../src/aalmetadatawritercontrol.h \
    ../../src/aalvideorenderercontrol.h \
    ../../src/storagemanager.h

SOURCES += tst_aalimagecapturecontrol.cpp \
    ../../src/aalimagecapturecontrol.cpp \
    aalcameraservice.cpp \
    aalimageencodercontrol.cpp \
    ../stubs/aalmetadatawritercontrol_stub.cpp \
    aalvideorenderercontrol.cpp \
    storagemanager.cpp

check.depends = $${TARGET}
check.commands = ./$${TARGET}
QMAKE_EXTRA_TARGETS += check
