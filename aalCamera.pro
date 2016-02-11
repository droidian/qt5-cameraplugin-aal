include(coverage.pri)

TEMPLATE = subdirs

SUBDIRS += \
    src \
    unittests
OTHER_FILES += .qmake.conf
