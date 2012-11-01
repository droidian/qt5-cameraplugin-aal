imclude(../../coverage.pri)

CONFIG += testcase
TARGET = tst_storagemanager

QT += testlib

SOURCES += tst_storagemanager.cpp \
    ../../src/storagemanager.cpp

INCLUDEPATH += ../../src
