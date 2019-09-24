/****************************************************************************
**
** Copyright (C) 2013 Aaron McCarthy <mccarthy.aaron@gmail.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
** 2015.4.4
** Adapted for use with QGroundControl
**
** Gus Grubba <mavlink@grubba.com>
**
****************************************************************************/

#include "QGCMapEngine.h"
#include "QGeoTiledMappingManagerEngineQGC.h"
#include "QGeoTileFetcherQGC.h"

#include <QtLocation/private/qgeocameracapabilities_p.h>
#include <QtLocation/private/qgeomaptype_p.h>
#if QT_VERSION < 0x050500
#include <QtLocation/private/qgeotiledmapdata_p.h>
#else
#include <QtLocation/private/qgeotiledmap_p.h>
#if QT_VERSION >= 0x050600
#include <QtLocation/private/qgeofiletilecache_p.h>
#else
#include <QtLocation/private/qgeotilecache_p.h>
#endif
#endif

#include <QDir>
#include <QStandardPaths>

#define PLGName "VGMapControl"
#if QT_VERSION >= 0x050500
//-----------------------------------------------------------------------------
QGeoTiledMapQGC::QGeoTiledMapQGC(QGeoTiledMappingManagerEngine *engine, QObject *parent)
    : QGeoTiledMap(engine, parent)
{
}
#endif

//-----------------------------------------------------------------------------
QGeoTiledMappingManagerEngineQGC::QGeoTiledMappingManagerEngineQGC(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString)
:   QGeoTiledMappingManagerEngine()
{
    QGeoCameraCapabilities cameraCaps;
    cameraCaps.setMinimumZoomLevel(2.0);
    cameraCaps.setMaximumZoomLevel(MAX_MAP_ZOOM);
    cameraCaps.setSupportsBearing(true);

    setCameraCapabilities(cameraCaps);
    setTileSize(QSize(256, 256));

    QList<QGeoMapType> mapTypes;
#ifndef QGC_NO_GOOGLE_MAPS
#if QT_VERSION >= 0x050B00
    mapTypes << QGeoMapType(QGeoMapType::StreetMap,       "Google Street Map",     "", false,  false,  UrlFactory::GoogleMap, PLGName, cameraCaps, parameters);
    mapTypes << QGeoMapType(QGeoMapType::SatelliteMapDay, "Google Satellite Map",  "", false,  false,  UrlFactory::GoogleSatellite, PLGName, cameraCaps, parameters);
    mapTypes << QGeoMapType(QGeoMapType::TerrainMap,      "Google Terrain Map",    "", false,  false,  UrlFactory::GoogleTerrain, PLGName, cameraCaps, parameters);
#else
    mapTypes << QGeoMapType(QGeoMapType::StreetMap, "Google Street Map", "", false, false, UrlFactory::GoogleMap);
    mapTypes << QGeoMapType(QGeoMapType::SatelliteMapDay, "Google Satellite Map", "", false, false, UrlFactory::GoogleSatellite);
    mapTypes << QGeoMapType(QGeoMapType::TerrainMap, "Google Terrain Map", "", false, false, UrlFactory::GoogleTerrain);
#endif
#endif//!QGC_NO_GOOGLE_MAPS
#if QT_VERSION >= 0x050B00
    // Bing
    mapTypes << QGeoMapType(QGeoMapType::StreetMap,       "Bing Street Map",      "", false,  false,  (int)UrlFactory::BingMap, PLGName, cameraCaps, parameters);
    mapTypes << QGeoMapType(QGeoMapType::SatelliteMapDay, "Bing Satellite Map",   "", false,  false,  (int)UrlFactory::BingSatellite, PLGName, cameraCaps, parameters);
    mapTypes << QGeoMapType(QGeoMapType::HybridMap,       "Bing Hybrid Map",      "", false,  false,  (int)UrlFactory::BingHybrid, PLGName, cameraCaps, parameters);

    // Baidu
    mapTypes << QGeoMapType(QGeoMapType::StreetMap,       "Baidu Street Map",     "", false,  false,  (int)UrlFactory::BaiduMap, PLGName, cameraCaps, parameters);
    mapTypes << QGeoMapType(QGeoMapType::SatelliteMapDay, "Baidu Satellite Map",  "", false,  false,  (int)UrlFactory::BaiduSatellite, PLGName, cameraCaps, parameters);
    mapTypes << QGeoMapType(QGeoMapType::HybridMap,       "Baidu Hybrid Map",     "", false,  false,    (int)UrlFactory::BaiduHybrid, PLGName, cameraCaps, parameters);

    // AMap（高德）
    mapTypes << QGeoMapType(QGeoMapType::StreetMap,       "AMap Street Map",      "", false,  false,  (int)UrlFactory::AMap, PLGName, cameraCaps, parameters);
    mapTypes << QGeoMapType(QGeoMapType::SatelliteMapDay, "AMap Satellite Map",   "", false,  false,  (int)UrlFactory::AMapSatelliteMap, PLGName, cameraCaps, parameters);
    mapTypes << QGeoMapType(QGeoMapType::HybridMap,       "AMap Hybrid Map",      "", false,  false,  (int)UrlFactory::AMapHybridMap, PLGName, cameraCaps, parameters);
#else
    mapTypes << QGeoMapType(QGeoMapType::StreetMap, "Bing Street Map", "", false, false, (int)UrlFactory::BingMap);
    mapTypes << QGeoMapType(QGeoMapType::SatelliteMapDay, "Bing Satellite Map", "", false, false, (int)UrlFactory::BingSatellite);
    mapTypes << QGeoMapType(QGeoMapType::HybridMap, "Bing Hybrid Map", "", false, false, (int)UrlFactory::BingHybrid);

    // Baidu
    mapTypes << QGeoMapType(QGeoMapType::StreetMap, "Baidu Street Map", "", false, false, (int)UrlFactory::BaiduMap);
    mapTypes << QGeoMapType(QGeoMapType::SatelliteMapDay, "Baidu Satellite Map", "", false, false, (int)UrlFactory::BaiduSatellite);
    mapTypes << QGeoMapType(QGeoMapType::HybridMap, "Baidu Hybrid Map", "", false, false, (int)UrlFactory::BaiduHybrid);

    // AMap（高德）
    mapTypes << QGeoMapType(QGeoMapType::StreetMap, "AMap Street Map", "", false, false, (int)UrlFactory::AMap);
    mapTypes << QGeoMapType(QGeoMapType::SatelliteMapDay, "AMap Satellite Map", "", false, false, (int)UrlFactory::AMapSatelliteMap);
    mapTypes << QGeoMapType(QGeoMapType::HybridMap, "AMap Hybrid Map", "", false, false, (int)UrlFactory::AMapHybridMap);
#endif
    /* See: https://wiki.openstreetmap.org/wiki/Tile_usage_policy
    mapTypes << QGeoMapType(QGeoMapType::StreetMap,         "Open Street Map",          "Open Street map",              false, false, UrlFactory::OpenStreetMap);
    */

    setSupportedMapTypes(mapTypes);
    //-- Users (QML code) can define a different user agent
    if (parameters.contains(QStringLiteral("useragent")))
        getQGCMapEngine()->setUserAgent(parameters.value(QStringLiteral("useragent")).toString().toLatin1());

#if QT_VERSION >= 0x050500
    _setCache(parameters);
#endif

    setTileFetcher(new QGeoTileFetcherQGC(this));

    *error = QGeoServiceProvider::NoError;
    errorString->clear();

#if QT_VERSION >= 0x050500
    if (parameters.contains(QStringLiteral("mapping.copyright")))
        m_customCopyright = parameters.value(QStringLiteral("mapping.copyright")).toString().toLatin1();
#endif
}

//-----------------------------------------------------------------------------
QGeoTiledMappingManagerEngineQGC::~QGeoTiledMappingManagerEngineQGC()
{
}

#if QT_VERSION < 0x050500

//-----------------------------------------------------------------------------
QGeoMapData *QGeoTiledMappingManagerEngineQGC::createMapData()
{
    return new QGeoTiledMapData(this, 0);
}

#else

//-----------------------------------------------------------------------------
QGeoMap*
QGeoTiledMappingManagerEngineQGC::createMap()
{
    return new QGeoTiledMapQGC(this);
}

//-----------------------------------------------------------------------------
QString
QGeoTiledMappingManagerEngineQGC::customCopyright() const
{
    return m_customCopyright;
}

#endif

#if QT_VERSION >= 0x050500
//-----------------------------------------------------------------------------
void
QGeoTiledMappingManagerEngineQGC::_setCache(const QVariantMap &parameters)
{
    QString cacheDir;
    if (parameters.contains(QStringLiteral("mapping.cache.directory")))
        cacheDir = parameters.value(QStringLiteral("mapping.cache.directory")).toString();
    else {
        cacheDir = getQGCMapEngine()->getCachePath();
        if(!QFileInfo(cacheDir).exists()) {
            if(!QDir::root().mkpath(cacheDir)) {
                qWarning() << "Could not create mapping disk cache directory: " << cacheDir;
                cacheDir = QDir::homePath() + QLatin1String("/.qgcmapscache/");
            }
        }
    }
    if(!QFileInfo(cacheDir).exists()) {
        if(!QDir::root().mkpath(cacheDir)) {
            qWarning() << "Could not create mapping disk cache directory: " << cacheDir;
            cacheDir.clear();
        }
    }
    //-- Memory Cache
    uint32_t memLimit = 0;
    if (parameters.contains(QStringLiteral("mapping.cache.memory.size"))) {
      bool ok = false;
      memLimit = parameters.value(QStringLiteral("mapping.cache.memory.size")).toString().toUInt(&ok);
      if (!ok)
          memLimit = 0;
    }
    if(!memLimit)
    {
        //-- Value saved in MB
        memLimit = getQGCMapEngine()->getMaxMemCache() * (1024 * 1024);
    }
    //-- It won't work with less than 1M of memory cache
    if(memLimit < 1024 * 1024)
        memLimit = 1024 * 1024;
    //-- On the other hand, Qt uses signed 32-bit integers. Limit to 1G to round it down (you don't need more than that).
    if(memLimit > 1024 * 1024 * 1024)
        memLimit = 1024 * 1024 * 1024;
    //-- Disable Qt's disk cache (sort of)
#if QT_VERSION >= 0x050600
    QAbstractGeoTileCache *pTileCache = new QGeoFileTileCache(cacheDir);
    setTileCache(pTileCache);
#else
    QGeoTileCache* pTileCache = createTileCacheWithDir(cacheDir);
#endif
    if(pTileCache)
    {
        //-- We're basically telling it to use 100k of disk for cache. It doesn't like
        //   values smaller than that and I could not find a way to make it NOT cache.
        pTileCache->setMaxDiskUsage(1024 * 100);
        pTileCache->setMaxMemoryUsage(memLimit);
    }
}
#endif
