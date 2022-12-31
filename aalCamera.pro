include(coverage.pri)

TEMPLATE = subdirs

SUBDIRS += \
    src \
    shadervideonode \
    sharedsignal
OTHER_FILES += .qmake.conf

src.depends += sharedsignal
shadervideonode.depends += sharedsignal