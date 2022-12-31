include(../coverage.pri)
TARGET = sharedsignal
TEMPLATE = lib
QT += gui

target.path += $$[QT_INSTALL_PLUGINS]/../..
INSTALLS = target

HEADERS += \
    media_signals.h

SOURCES += \
    media_signals.cpp

