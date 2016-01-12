include(../../coverage.pri)

TARGET = tst_aalvideodeviceselectorcontrol

QT += testlib multimedia opengl

LIBS += -L../mocks/aal -laal
INCLUDEPATH += ../../src
INCLUDEPATH += ../mocks/aal

HEADERS += ../../src/aalvideodeviceselectorcontrol.h \
    ../../src/aalcameracontrol.h \
    ../../src/aalcameraservice.h \
    ../../src/aalvideoencodersettingscontrol.h

SOURCES += tst_aalvideodeviceselectorcontrol.cpp \
    ../../src/aalvideodeviceselectorcontrol.cpp \
    aalcameraservice.cpp \
    aalimageencodercontrol.cpp \
    aalviewfindersettingscontrol.cpp \
    ../stubs/aalcameracontrol_stub.cpp \
    ../stubs/aalvideoencodersettingscontrol_stub.cpp

check.depends = $${TARGET}
check.commands = ./$${TARGET}
QMAKE_EXTRA_TARGETS += check
