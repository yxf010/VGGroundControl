/*=====================================================================

 QGroundControl Open Source Ground Control Station

 (c) 2009 - 2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>

 This file is part of the QGROUNDCONTROL project

 QGROUNDCONTROL is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 QGROUNDCONTROL is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.

 ======================================================================*/

/// @file
///     @author Gus Grubba <mavlink@grubba.com>

#include "VGMapEngineManager.h"
#include "QGCMapTileSet.h"
#include "QGCMapUrlEngine.h"

#include <QSettings>
#include <QStorageInfo>
#include <stdio.h>
#include <QQmlEngine>
#include <QtQml>

//QGC_LOGGING_CATEGORY(QGCMapEngineManagerLog, "QGCMapEngineManagerLog")

static const char* kQmlOfflineMapKeyName = "QGCOfflineMap";

//-----------------------------------------------------------------------------
VGMapEngineManager::VGMapEngineManager(/*QGCApplication* app*/)
    : QObject()
    , _topleftLat(0.0)
    , _topleftLon(0.0)
    , _bottomRightLat(0.0)
    , _bottomRightLon(0.0)
    , _minZoom(0)
    , _maxZoom(0)
    , _setID(UINT64_MAX)
    , _freeDiskSpace(0)
    , _diskSpace(0)
{

}

//-----------------------------------------------------------------------------
VGMapEngineManager::~VGMapEngineManager()
{
    _tileSets.clear();
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::setToolbox(/*QGCToolbox *toolbox*/)
{
   //QGCTool::setToolbox(toolbox);
   QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
   qmlRegisterUncreatableType<VGMapEngineManager>("VGGroundControl", 1, 0, "VGMapEngineManager", "Reference only");
   connect(getQGCMapEngine(), &QGCMapEngine::updateTotals, this, &VGMapEngineManager::_updateTotals);
   _updateDiskFreeSpace();
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::updateForCurrentView(double lon0, double lat0, double lon1, double lat1, int minZoom, int maxZoom, const QString& mapName)
{
    UrlFactory::MapType mapType = QGCMapEngine::getTypeFromName(mapName);

    _topleftLat     = lat0;
    _topleftLon     = lon0;
    _bottomRightLat = lat1;
    _bottomRightLon = lon1;
    _minZoom        = minZoom;
    _maxZoom        = maxZoom;
    _totalSet.clear();
    for(int z = minZoom; z <= maxZoom; z++) {
        QGCTileSet set = QGCMapEngine::getTileCount(z, lon0, lat0, lon1, lat1, mapType);
        _totalSet += set;
    }
    emit tileX0Changed();
    emit tileX1Changed();
    emit tileY0Changed();
    emit tileY1Changed();
    emit tileCountChanged();
    emit tileSizeChanged();

    //qCDebug(QGCMapEngineManagerLog) << "updateForCurrentView" << lat0 << lon0 << lat1 << lon1 << minZoom << maxZoom;
}

//-----------------------------------------------------------------------------
QString
VGMapEngineManager::tileCountStr()
{
    return QGCMapEngine::numberToString(_totalSet.tileCount);
}

//-----------------------------------------------------------------------------
QString
VGMapEngineManager::tileSizeStr()
{
    return QGCMapEngine::bigSizeToString(_totalSet.tileSize);
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::loadTileSets()
{
    if(_tileSets.count()) {
        _tileSets.clear();
        emit tileSetsChanged();
    }
    QGCFetchTileSetTask* task = new QGCFetchTileSetTask();
    connect(task, &QGCFetchTileSetTask::tileSetFetched, this, &VGMapEngineManager::_tileSetFetched);
    connect(task, &QGCMapTask::error, this, &VGMapEngineManager::taskError);
    getQGCMapEngine()->addTask(task);
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::_tileSetFetched(QGCCachedTileSet* tileSet)
{
    //-- A blank (default) type means it uses various types and not just one
    if(tileSet->type() == UrlFactory::Invalid) {
        tileSet->setMapTypeStr("Various");
    }
    _tileSets.append(tileSet);
    //tileSet->setManager(this);
    emit tileSetsChanged();
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::startDownload(const QString& name, const QString& mapType)
{
    if(_totalSet.tileSize) {
        QGCCachedTileSet* set = new QGCCachedTileSet(name);
        set->setMapTypeStr(mapType);
        set->setTopleftLat(_topleftLat);
        set->setTopleftLon(_topleftLon);
        set->setBottomRightLat(_bottomRightLat);
        set->setBottomRightLon(_bottomRightLon);
        set->setMinZoom(_minZoom);
        set->setMaxZoom(_maxZoom);
        set->setTotalTileSize(_totalSet.tileSize);
        set->setTotalTileCount(_totalSet.tileCount);
        set->setType(QGCMapEngine::getTypeFromName(mapType));
        QGCCreateTileSetTask* task = new QGCCreateTileSetTask(set);
        //-- Create Tile Set (it will also create a list of tiles to download)
        connect(task, &QGCCreateTileSetTask::tileSetSaved, this, &VGMapEngineManager::_tileSetSaved);
        connect(task, &QGCMapTask::error, this, &VGMapEngineManager::taskError);
        getQGCMapEngine()->addTask(task);
    } else {
        qWarning() <<  "VGMapEngineManager::startDownload() No Tiles to save";
    }
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::_tileSetSaved(QGCCachedTileSet *set)
{
    //qCDebug(QGCMapEngineManagerLog) << "New tile set saved (" << set->name() << "). Starting download...";
    _tileSets.append(set);
    emit tileSetsChanged();
    //-- Start downloading tiles
    set->createDownloadTask();
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::saveSetting (const QString& key, const QString& value)
{
    QSettings settings;
    settings.beginGroup(kQmlOfflineMapKeyName);
    settings.setValue(key, value);
}

//-----------------------------------------------------------------------------
QString
VGMapEngineManager::loadSetting (const QString& key, const QString& defaultValue)
{
    QSettings settings;
    settings.beginGroup(kQmlOfflineMapKeyName);
    return settings.value(key, defaultValue).toString();
}

//-----------------------------------------------------------------------------
QStringList
VGMapEngineManager::mapList()
{
    return getQGCMapEngine()->getMapNameList();
}

//-----------------------------------------------------------------------------
QString
VGMapEngineManager::mapboxToken()
{
    return getQGCMapEngine()->getMapBoxToken();
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::setMapboxToken(QString token)
{
    getQGCMapEngine()->setMapBoxToken(token);
}

//-----------------------------------------------------------------------------
quint32
VGMapEngineManager::maxMemCache()
{
    return getQGCMapEngine()->getMaxMemCache();
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::setMaxMemCache(quint32 size)
{
    getQGCMapEngine()->setMaxMemCache(size);
}

//-----------------------------------------------------------------------------
quint32
VGMapEngineManager::maxDiskCache()
{
    return getQGCMapEngine()->getMaxDiskCache();
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::setMaxDiskCache(quint32 size)
{
    getQGCMapEngine()->setMaxDiskCache(size);
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::deleteTileSet(QGCCachedTileSet* tileSet)
{
    //qCDebug(QGCMapEngineManagerLog) << "Deleting tile set " << tileSet->name();
    //-- If deleting default set, delete it all
    if(tileSet->defaultSet()) {
        for(int i = 0; i < _tileSets.count(); i++ ) {
            QGCCachedTileSet* set = qobject_cast<QGCCachedTileSet*>(_tileSets.get(i));
            Q_ASSERT(set);
            set->setDeleting(true);
        }
        QGCResetTask* task = new QGCResetTask();
        connect(task, &QGCResetTask::resetCompleted, this, &VGMapEngineManager::_resetCompleted);
        connect(task, &QGCMapTask::error, this, &VGMapEngineManager::taskError);
        getQGCMapEngine()->addTask(task);
    } else {
        tileSet->setDeleting(true);
        QGCDeleteTileSetTask* task = new QGCDeleteTileSetTask(tileSet->setID());
        connect(task, &QGCDeleteTileSetTask::tileSetDeleted, this, &VGMapEngineManager::_tileSetDeleted);
        connect(task, &QGCMapTask::error, this, &VGMapEngineManager::taskError);
        getQGCMapEngine()->addTask(task);
    }
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::_resetCompleted()
{
    //-- Reload sets
    loadTileSets();
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::_tileSetDeleted(quint64 setID)
{
    //-- Tile Set successfully deleted
    QGCCachedTileSet* setToDelete = NULL;
    int i = 0;
    for(i = 0; i < _tileSets.count(); i++ ) {
        QGCCachedTileSet* set = qobject_cast<QGCCachedTileSet*>(_tileSets.get(i));
        Q_ASSERT(set);
        if (set->setID() == setID) {
            setToDelete = set;
            break;
        }
    }
    if(setToDelete) {
        _tileSets.removeAt(i);
        delete setToDelete;
    }
    emit tileSetsChanged();
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::taskError(QGCMapTask::TaskType type, const QString &error)
{
    QString task;
    switch(type) {
    case QGCMapTask::taskFetchTileSets:
        task = "Fetch Tile Set";
        break;
    case QGCMapTask::taskCreateTileSet:
        task = "Create Tile Set";
        break;
    case QGCMapTask::taskGetTileDownloadList:
        task = "Get Tile Download List";
        break;
    case QGCMapTask::taskUpdateTileDownloadState:
        task = "Update Tile Download Status";
        break;
    case QGCMapTask::taskDeleteTileSet:
        task = "Delete Tile Set";
        break;
    case QGCMapTask::taskReset:
        task = "Reset Tile Sets";
        break;
    case QGCMapTask::taskImportTileSet:
        task = "import offline maps";
        break;
    default:
        task = "Database Error";
        break;
    }
    QString serror = "Error in task: " + task;
    serror += "\nError description:\n";
    serror += error;
    qWarning() << "VGMapEngineManager::_taskError()";
    setErrorMessage(serror);
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::_updateTotals(quint32 totaltiles, quint64 totalsize, quint32 defaulttiles, quint64 defaultsize)
{
    for(int i = 0; i < _tileSets.count(); i++ ) {
        QGCCachedTileSet* set = qobject_cast<QGCCachedTileSet*>(_tileSets.get(i));
        Q_ASSERT(set);
        if (set->defaultSet()) {
            set->setSavedTileSize(totalsize);
            set->setSavedTileCount(totaltiles);
            set->setTotalTileCount(defaulttiles);
            set->setTotalTileSize(defaultsize);
            return;
        }
    }
    _updateDiskFreeSpace();
}

//-----------------------------------------------------------------------------
bool
VGMapEngineManager::findName(const QString& name)
{
    for(int i = 0; i < _tileSets.count(); i++ ) {
        QGCCachedTileSet* set = qobject_cast<QGCCachedTileSet*>(_tileSets.get(i));
        Q_ASSERT(set);
        if (set->name() == name) {
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
QString
VGMapEngineManager::getUniqueName()
{
    QString test = "Tile Set ";
    QString name;
    int count = 1;
    while (true) {
        name = test;
        name += QString().sprintf("%03d", count++);
        if(!findName(name))
            return name;
    }
}

//-----------------------------------------------------------------------------
void
VGMapEngineManager::_updateDiskFreeSpace()
{
    QString path = getQGCMapEngine()->getCachePath();
    if(!path.isEmpty()) {
        QStorageInfo info(path);
        quint32 total = (quint32)(info.bytesTotal() / 1024);
        quint32 free  = (quint32)(info.bytesFree()  / 1024);
        //qCDebug(QGCMapEngineManagerLog) << info.rootPath() << "has" << free << "Mbytes available.";
        if(_freeDiskSpace != free) {
            _freeDiskSpace = free;
            _diskSpace = total;
            emit freeDiskSpaceChanged();
        }
    }
}

void VGMapEngineManager::setExportMaps(int tileSetID)
{
    QGCExportTileSetTask* task = new QGCExportTileSetTask(tileSetID);
    qRegisterMetaType<QList<QGCCacheTile*>>("QList<QGCCacheTile*>");
    connect(task, &QGCExportTileSetTask::tileSetExported, this, &VGMapEngineManager::_tileSetExported);
    connect(task, &QGCExportTileSetTask::error, this, &VGMapEngineManager::taskError);
    getQGCMapEngine()->addTask(task);
}

void VGMapEngineManager::_tileSetExported(const QString &setID, const QString &setName, const QList<QGCCacheTile *> &tiles)
{
    emit tileSetExportedResult(setID, setName, tiles);
}

void VGMapEngineManager::setImportMaps(const QString &setId, const QString &setTileName, const QList<QGCCacheTile *> &lstTiles)
{
    QGCImportTileSetTask* task = new QGCImportTileSetTask(setId.toULongLong());
    qRegisterMetaType<QList<QGCCacheTile*>>("QList<QGCCacheTile*>");
    //connect(task, SIGNAL(tileSetExported(QString, QString, QList<QGCCacheTile*>)), this, SLOT(_tileSetExported(QString, QString, QList<QGCCacheTile*>)));
    connect(task, &QGCMapTask::error, this, &VGMapEngineManager::taskError);
    task->setTileSetImported(setTileName, lstTiles);
    getQGCMapEngine()->addTask(task);
}

void VGMapEngineManager::_tileSetImportFinished(int setId)
{
    qDebug() << "finished to import offline maps" << setId;
}
