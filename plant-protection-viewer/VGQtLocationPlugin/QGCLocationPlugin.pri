QT  += location-private positioning-private

HEADERS += \
    $$PWD/QGCMapEngine.h \
    $$PWD/QGCMapEngineData.h \
    $$PWD/QGCMapTileSet.h \
    $$PWD/QGCMapUrlEngine.h \
    $$PWD/QGCTileCacheWorker.h \
    $$PWD/QGeoCodeReplyQGC.h \
    $$PWD/QGeoCodingManagerEngineQGC.h \
    $$PWD/QGeoMapReplyQGC.h \
    $$PWD/QGeoServiceProviderPluginQGC.h \
    $$PWD/QGeoTileFetcherQGC.h \
    $$PWD/QGeoTiledMappingManagerEngineQGC.h \
    $$PWD/VGMapGlobalFunction.h

SOURCES += \
    $$PWD/QGCMapEngine.cpp \
    $$PWD/QGCMapTileSet.cpp \
    $$PWD/QGCMapUrlEngine.cpp \
    $$PWD/QGCTileCacheWorker.cpp \
    $$PWD/QGeoCodeReplyQGC.cpp \
    $$PWD/QGeoCodingManagerEngineQGC.cpp \
    $$PWD/QGeoMapReplyQGC.cpp \
    $$PWD/QGeoServiceProviderPluginQGC.cpp \
    $$PWD/QGeoTileFetcherQGC.cpp \
    $$PWD/QGeoTiledMappingManagerEngineQGC.cpp \
    $$PWD/VGMapGlobalFunction.cpp

OTHER_FILES += \
    $$PWD/vg_maps_plugin.json
