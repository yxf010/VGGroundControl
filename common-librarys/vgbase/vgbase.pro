TEMPLATE = lib

include(../libs.pri)

QT -= gui
QT += core

DEFINES += VGBASE_LIBRARY

INCLUDEPATH += .

INCLUDEPATH += \
    $$SOURCEDIR/vgbase/src

WindowsBuild {
    INCLUDEPATH += $$SOURCEDIR/libs/lib/msinttypes
}

SOURCES += \
    $$SOURCEDIR/vgbase/src/base.cc \

HEADERS += \
    $$SOURCEDIR/vgbase/src/base.h \
    $$SOURCEDIR/vgbase/src/vgbase_global.h

DESTDIR = $${OUT_ROOT}
