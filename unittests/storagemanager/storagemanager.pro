include(../../coverage.pri)

TARGET = tst_storagemanager

QT += testlib

CONFIG += link_pkgconfig
PKGCONFIG += exiv2

SOURCES += tst_storagemanager.cpp \
    ../../src/storagemanager.cpp

INCLUDEPATH += ../../src

check.depends = $${TARGET}
check.commands = ./$${TARGET}
QMAKE_EXTRA_TARGETS += check
