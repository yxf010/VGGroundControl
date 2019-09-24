TEMPLATE = lib

include(../libs.pri)

QT       -= gui

DEFINES += VGLOG_LIBRARY
DEFINES += QS_LOG_LINE_NUMBERS

INCLUDEPATH += .

INCLUDEPATH += \
    include \
    $$SOURCEDIR/vglog/include

SOURCES += \
    $$SOURCEDIR/vglog/src/QsLog.cpp \
    $$SOURCEDIR/vglog/src/QsLogDest.cpp \
    $$SOURCEDIR/vglog/src/QsLogDestConsole.cpp \
    $$SOURCEDIR/vglog/src/QsLogDestFile.cpp

HEADERS += \
    $$SOURCEDIR/vglog/include/QsLog.h \
    $$SOURCEDIR/vglog/include/QsLogDest.h \
    $$SOURCEDIR/vglog/include/QsLogDestConsole.h \
    $$SOURCEDIR/vglog/include/QsLogDestFile.h \
    $$SOURCEDIR/vglog/include/QsLogDisableForThisFile.h \
    $$SOURCEDIR/vglog/include/QsLogLevel.h \
    $$SOURCEDIR/vglog/include/vglog_global.h

DESTDIR = $${OUT_ROOT}
