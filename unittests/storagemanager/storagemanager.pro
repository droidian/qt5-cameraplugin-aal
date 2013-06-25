include(../../coverage.pri)

TARGET = tst_storagemanager

QT += testlib

SOURCES += tst_storagemanager.cpp \
    ../../src/storagemanager.cpp

INCLUDEPATH += ../../src

check.depends = $${TARGET}
check.commands = ./$${TARGET}
QMAKE_EXTRA_TARGETS += check
