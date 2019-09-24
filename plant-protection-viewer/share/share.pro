TEMPLATE = lib

QT -= gui core

DEFINES += SHARE_LIBRARY
CONFIG += x11 dll

INCLUDEPATH += .

SOURCES += cppsrc/ShareFunction.cpp \
    cppsrc/RouteStruct.cpp \
    cppsrc/RoutePlanning.cpp

HEADERS += share_global.h \
    ShareFunction.h \
    RouteStruct.h \
    RoutePlanning.h


!iOSBuild {
    OBJECTS_DIR  = $${OUT_PWD}/obj
}

CONFIG(debug, debug|release) {
    DESTDIR = $${OUT_PWD}/../../output/debug/
}
else:CONFIG(release, debug|release) {
    DESTDIR = $${OUT_PWD}/../../output/release/
}
