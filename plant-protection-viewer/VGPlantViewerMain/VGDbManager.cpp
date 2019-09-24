#include "VGDbManager.h"
#include <QDebug>
#include <QGeoCoordinate>
#include <QDateTime>
#include "VGLandInformation.h"
#include "VGLandPolygon.h"
#include "VGCoordinate.h"
#include "VGLandBoundary.h"
#include "VGFlyRoute.h"
#include "VGOutline.h"
#include "VGApplication.h"
#include "VGGlobalFun.h"

#define KSession "VGDBSession"
enum {
    DB_TABLEERROR = -6,
    DB_QUERYERROR = -5,
    DB_DELETEERROR = -4,
    DB_UPDATEERROR = -3,
    DB_ADDERROR = -2,
    DB_INITIALERROR=-1,
    DB_OPENERROR = 0,
    DB_INITIAL = 1,
    DB_OEPN,
    DB_ADD,
    DB_UPDATE,
    DB_DELETE,
    DB_QUERY,
};

static QString describeMap2Condition(const DescribeMap &des)
{
    QString strRet;
    DescribeMap::const_iterator itr = des.begin();
    for (; itr != des.end(); ++itr)
    {
        if (itr.key() == "table")
            continue;

        QString var = VGDbManager::toString(itr.value());
        QString strTmp = QString("%1=\"%2\"").arg(itr.key()).arg(var);

        if (strRet.isEmpty())
            strRet = strTmp;
        else
            strRet += " AND " + strTmp;
    }
    if (strRet.isEmpty())
        return strRet;
    return " where " + strRet;
}
////////////////////////////////////////////////////////////////////////////////////////
//VGDbManager
////////////////////////////////////////////////////////////////////////////////////////
VGDbManager::VGDbManager(QObject *parent) : QObject(parent)
{
    m_dbase = NULL;
    m_bDBOpened = false;
    qRegisterMetaType<DescribeMap>("DescribeMap");
    connect(this, &VGDbManager::_sigCreatDb, this, &VGDbManager::createDb);
    connect(this, &VGDbManager::_saveItem, this, &VGDbManager::onSaveItem);
    connect(this, &VGDbManager::_deleteItem, this, &VGDbManager::onDeleteItem);
    connect(this, &VGDbManager::_sigQueryInfo, this, &VGDbManager::queryItems);
    connect(this, &VGDbManager::_update, this, &VGDbManager::onUpdateItem);
}

QString VGDbManager::toString(const QVariant &var)
{
    switch (var.type())
    {
    case QVariant::String:
        return var.toString();
    case QVariant::Int:
        return QString::number(var.toInt());
    case QVariant::LongLong:
        return QString::number(var.toLongLong());
    case QVariant::Bool:
        return var.toBool()?"1":"0";
    default:
        break;
    }
    return QString();
}

const QString &VGDbManager::getDBOperateDescribe(int res)
{
    static QMap<int, QString> sMapDsc;
    if (sMapDsc.isEmpty())
    {
        sMapDsc[DB_TABLEERROR] = QString::fromStdWString(L"创建表错误!");
        sMapDsc[DB_QUERYERROR] = QString::fromStdWString(L"查询数据库错误!");
        sMapDsc[DB_DELETEERROR] = QString::fromStdWString(L"删除数据库记录失败!");
        sMapDsc[DB_UPDATEERROR] = QString::fromStdWString(L"更新数据库失败!");
        sMapDsc[DB_ADDERROR] = QString::fromStdWString(L"添加数据失败!");
        sMapDsc[DB_INITIALERROR] = QString::fromStdWString(L"打开数据库文件失败!");
        sMapDsc[DB_OPENERROR] = QString::fromStdWString(L"数据库未打开!");
        sMapDsc[DB_INITIAL] = QString::fromStdWString(L"数据库初始化成功!");
        sMapDsc[DB_OEPN] = QString::fromStdWString(L"数据库打开成功!");
        sMapDsc[DB_ADD] = QString::fromStdWString(L"添加数据完成!");
        sMapDsc[DB_UPDATE] = QString::fromStdWString(L"更新数据完成!");
        sMapDsc[DB_DELETE] = QString::fromStdWString(L"删除数据库记录完成!");
        sMapDsc[DB_QUERY] = QString::fromStdWString(L"查询数据库完成!");
    }

    QMap<int, QString>::iterator itr = sMapDsc.find(res);
    if (itr != sMapDsc.end())
        return itr.value();

    static QString sStr;
    return  sStr;
}

bool VGDbManager::isDBOpened()
{
    return m_bDBOpened;
}

void VGDbManager::CreateDb(const QString &dbName)
{
    emit _sigCreatDb(dbName);
}

void VGDbManager::SaveItemSafe(MapAbstractItem *item)
{
    if (isDBOpened())
        emit _saveItem(item);
}

void VGDbManager::DeleteItemSafe(MapAbstractItem *item)
{
    if (!item || !item->IsSaveLocal())
        return;

    DescribeMap dsc;
    VGGlobalFunc::initialTableName(dsc, item->ItemType());
    VGGlobalFunc::initialItemDescribe(dsc, *item, false);

    emit _deleteItem(dsc);
}

void VGDbManager::QueryItems(MapAbstractItem::MapItemType tp, const MapAbstractItem *parent)
{
    DescribeMap dsc;
    VGGlobalFunc::initialTableName(dsc, tp);
    if (parent)
        VGGlobalFunc::initialItemDescribe(dsc, *parent);

    if (dsc.find("table") != dsc.end())
        emit _sigQueryInfo(dsc);
}

void VGDbManager::UpdateDb(const MapAbstractItem *item)
{
    emit _update(item);
}

bool VGDbManager::createDb(const QString &dbName)
{
    int nRet = 0;
    if(dbName.isEmpty())
        return false;

    m_dbase = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", KSession));
    m_dbase->setDatabaseName(dbName);
    m_dbase->setConnectOptions("QSQLITE_ENABLE_SHARED_CACHE");
    if (m_dbase->open())
    {
        m_dbBaseName = dbName;
        {
            if(createTables())
                nRet = 1;
        }
    }
    else
    {
        emit sigSqlResult(DB_INITIALERROR);
        nRet = 0;
        return false;
    }

    emit sigSqlResult(DB_INITIAL);

    if(nRet == 1)
        m_bDBOpened = true;

    return true;
}

bool VGDbManager::isExistTable(QString tableName)
{
    if(!m_dbase)
        return false;

    QString strSql = QString("SELECT COUNT(*) FROM sqlite_master WHERE type = \"table\" AND name = \"%1\"").arg(tableName);
    QSqlQuery query(*m_dbase);
    if(query.exec(strSql))
    {
        if (query.next())
            return query.value(0).toInt()>0;        
    }

    return false;
}

void VGDbManager::_saveLand(VGLandInformation &land)
{
    QString coors;
    foreach(VGCoordinate *itr, land.m_coorsSurvey)
    {
        coors += coors.isEmpty() ? coordinateToString(*itr) : ";" + coordinateToString(*itr);
    }

    QString strSql = QString::fromLocal8Bit("insert into VGLandSurveys(actualId, surveyUser, owner, address, "
        "surveyTime, surveyPrecision, uploaded, plantCoordinates, surveyType) "
        "VALUES (:actualId, :surveyUser, :owner, :address, :surveyTime, "
        ":surveyPrecision, :uploaded, :plantCoordinates, :surveyType)");

    QSqlQuery query(*m_dbase);
    query.prepare(strSql);
    query.bindValue(":actualId", land.actualSurveyId());
    query.bindValue(":surveyUser", land.userId());
    query.bindValue(":owner", land.m_owner.ToString());
    query.bindValue(":address", land.GetAddress());
    query.bindValue(":surveyTime", land.surveyTime());
    query.bindValue(":surveyPrecision", land.precision());
    query.bindValue(":uploaded", land.IsUpLoaded());
    query.bindValue(":plantCoordinates", coors);
    query.bindValue(":surveyType", land.m_typeSurvey);

    if (!query.exec())
    {
        emit sigSqlResult(DB_ADDERROR);
        return;
    }

    land.SetSaveLocal(true);
    emit sigSqlResult(DB_ADD);
}

void VGDbManager::_saveBoundary(VGLandBoundary &boundary)
{
    VGLandInformation *land = boundary.GetBelongedLand();
	if (!land)
		return;

    QString strOutlines;
    QString strBlocklines;
    QString strSafeArea;
    foreach(VGLandPolygon *itr, boundary.m_polygons)
    {
        if (itr->GetId() == VGLandPolygon::Boundary)
            strOutlines = polygon2String(*itr);
        else
            strBlocklines += strBlocklines.isEmpty() ? polygon2String(*itr) : "#" + polygon2String(*itr);
    }
    if (boundary.m_safeArea)
        strSafeArea = polygon2String(*boundary.m_safeArea);

    QString  strSql = QString::fromLocal8Bit("insert into VGLandBuandarys(boundaryId, surveyUser, surveyTime, editTime, "
        "uploaded, describe, boundary, blocksCoordinates, safaArea) "
        "VALUES (:boundaryId, :surveyUser, :surveyTime, :editTime, :uploaded, "
        ":describe, :boundary, :blocksCoordinates, :safaArea)");

    if (boundary.m_time == 0)
        boundary.m_time = QDateTime::currentMSecsSinceEpoch();

    QSqlQuery query(*m_dbase);
    query.prepare(strSql);
    query.bindValue(":boundaryId", boundary.m_cloudID);
    query.bindValue(":surveyUser", land->m_userId);
    query.bindValue(":surveyTime", land->m_time);
	query.bindValue(":editTime", boundary.m_time);
    query.bindValue(":uploaded", boundary.m_bUploaded);
    query.bindValue(":describe", boundary.m_strDescribe);
    query.bindValue(":boundary", strOutlines.replace("nan", "0"));
    query.bindValue(":blocksCoordinates", strBlocklines);
    query.bindValue(":safaArea", strSafeArea);

    if (!query.exec())
    {
        emit sigSqlResult(DB_ADDERROR);
        return;
    }

    boundary.SetSaveLocal(true);
    emit sigSqlResult(DB_ADD);
}

void VGDbManager::_saveRoute(VGFlyRoute &route)
{
    VGLandInformation *land = route.GetBelongedLand();
    VGLandBoundary *bdr = route.GetBelongedBoundary();
    if (!land || !bdr)
        return;

    QString strOutlines;
    QString strSupplies;

    if (VGOutline *itr = route.allMissionRoute())   
        strOutlines = polyline2String(*itr);

    QString  strSql = QString::fromLocal8Bit("insert into VGFlyRoutes(describe, surveyUser, surveyTime, editTime, routeTime, uploaded, "
        "operationHeight, routeInfo, totalVoyage, transVoyage, operateVoyage, routes, supplies) "
        "VALUES (:describe, :surveyUser, :surveyTime, :editTime, :routeTime, :uploaded, "
        ":operationHeight, :routeInfo, :totalVoyage, :transVoyage, :operateVoyage, :routes, :supplies)");

    if (route.m_time == 0)
        route.m_time = QDateTime::currentMSecsSinceEpoch();

    QSqlQuery query(*m_dbase);
    query.prepare(strSql);
    query.bindValue(":describe", route.m_strDescribe);
    query.bindValue(":surveyUser", land->m_userId);
    query.bindValue(":surveyTime", land->m_time);
    query.bindValue(":editTime", bdr->m_time);
    query.bindValue(":routeTime", route.m_time);
    query.bindValue(":uploaded", route.m_bUploaded);
    query.bindValue(":operationHeight", route.m_operationHeight);
    query.bindValue(":routeInfo", route.GetInfoString());
    query.bindValue(":totalVoyage", route.m_totalVoyage);
    query.bindValue(":transVoyage", route.m_transVoyage);
    query.bindValue(":operateVoyage", route.m_operateVoyage);
    query.bindValue(":routes", strOutlines);
    query.bindValue(":supplies", strSupplies);

    if (!query.exec())
    {
        emit sigSqlResult(DB_ADDERROR);
        return;
    }

    route.SetSaveLocal(true);
    emit sigSqlResult(DB_ADD);
}

void VGDbManager::_updateLand(VGLandInformation &land)
{
    DescribeMap dsc;
    VGGlobalFunc::initialItemDescribe(dsc, land, false);
    QString strId = land.actualSurveyId();
    if (strId.isEmpty())
        return;

    QString strSql = QString("update VGLandSurveys SET actualId=\"%1\",uploaded=\"%2\"").arg(strId).arg(land.IsUpLoaded()) + describeMap2Condition(dsc);
    QSqlQuery query(*m_dbase);
    if (!query.exec(strSql))
    {
        emit sigSqlResult(DB_UPDATEERROR);
        return;
    }

    emit sigSqlResult(DB_UPDATE);
}

void VGDbManager::_updateBoundary(VGLandBoundary &boundary)
{
    DescribeMap dsc;
    VGGlobalFunc::initialItemDescribe(dsc, boundary, false);
    QString strId = boundary.m_cloudID;
    if (strId.isEmpty())
        return;

    QString strSql = QString("update VGLandBuandarys SET boundaryId=\"%1\",uploaded=\"%2\"").arg(strId).arg(boundary.IsUpLoaded()) + describeMap2Condition(dsc);
    QSqlQuery query(*m_dbase);
    if (!query.exec(strSql))
    {
        emit sigSqlResult(DB_UPDATEERROR);
        return;
    }

    emit sigSqlResult(DB_UPDATE);
}

void VGDbManager::_updateRoute(VGFlyRoute &route)
{
    DescribeMap dsc;
    VGGlobalFunc::initialItemDescribe(dsc, route, false);
    QString strId = route.m_strActId;
    if (strId.isEmpty())
        return;

    QString strSql = QString("update VGFlyRoutes SET routeInfo=\"%1\",uploaded=\"%2\"").arg(route.GetInfoString()).arg(route.IsUpLoaded()) + describeMap2Condition(dsc);
    QSqlQuery query(*m_dbase);
    if (!query.exec(strSql))
    {
        emit sigSqlResult(DB_UPDATEERROR);
        return;
    }

    emit sigSqlResult(DB_UPDATE);
}

void VGDbManager::_processSurveys(QSqlQuery &query)
{
    if (!qvgApp->landManager())
        return;

    while (query.next())
    {
        VGLandInformation *infoLand = new VGLandInformation(this);
        infoLand->m_userId = query.value("surveyUser").toString();
        infoLand->m_time = query.value("surveyTime").toLongLong();
        infoLand->m_precision = query.value("surveyPrecision").toInt();
        infoLand->m_bUploaded = query.value("uploaded").toBool();
        infoLand->m_actualSurveyId = query.value("actualId").toString();
        infoLand->m_adress = query.value("address").toString();
        infoLand->m_owner.ParseString(query.value("owner").toString());
        infoLand->m_typeSurvey = (MapAbstractItem::SurveyType)query.value("surveyType").toInt();

        QString coors = query.value("plantCoordinates").toString();

        if (!coors.isEmpty())
        {
            foreach(const QString &str, coors.split(";", QString::SkipEmptyParts))
            {
                VGCoordinate *tmp = new VGCoordinate();
                if (initalCoordinateByString(*tmp, str))
                    infoLand->m_coorsSurvey.append(tmp);
                else
                    delete tmp;
            }
        }
        infoLand->m_bSaveLocal = true;
        emit sigQueryItem(infoLand);
    }
}

void VGDbManager::_processBoundary(QSqlQuery &query)
{
    if (!qvgApp->landManager())
        return;

    while (query.next())
    {
        VGLandBoundary *infoDes = new VGLandBoundary(this);
        DescribeMap dsc;

        dsc["surveyUser"] = query.value("surveyUser").toString();
        dsc["surveyTime"] = query.value("surveyTime").toLongLong();
        dsc["editTime"] = query.value("surveyTime").toLongLong();
        infoDes->m_cloudID = query.value("boundaryId").toString();
        infoDes->m_time = query.value("editTime").toLongLong();
        infoDes->m_bUploaded = query.value("uploaded").toBool();
        infoDes->m_strDescribe = query.value("describe").toString();

        QString strOutLine = query.value("boundary").toString();
        if (!strOutLine.isEmpty())
        {
            VGLandPolygon *tmp = new VGLandPolygon(infoDes, VGLandPolygon::Boundary);
            initalPolygonByString(*tmp, strOutLine);
            infoDes->m_polygons.append(tmp);
        }
        QString strBlocks = query.value("blocksCoordinates").toString();
        if (!strBlocks.isEmpty())
        {
            foreach(const QString &str, strBlocks.split("#", QString::SkipEmptyParts))
            {
                VGLandPolygon *tmp = new VGLandPolygon(infoDes, VGLandPolygon::BlockBoundary);
                if (initalPolygonByString(*tmp, str))
                    infoDes->m_polygons.append(tmp);
                else
                    delete tmp;
            }
        }
        QString strSafaArea = query.value("safaArea").toString();
        if (!strSafaArea.isEmpty())
        {
            VGLandPolygon *tmp = new VGLandPolygon(infoDes, VGLandPolygon::BlockBoundary);
            if (initalPolygonByString(*tmp, strSafaArea))
                infoDes->m_safeArea = tmp;
            else
                delete tmp;
        }
        infoDes->m_bSaveLocal = true;
        emit sigQueryItem(infoDes, dsc);
    }
}

void VGDbManager::_processRoute(QSqlQuery &query)
{
    if (!qvgApp->landManager())
        return;

    while (query.next())
    {
        VGFlyRoute *infoDes = new VGFlyRoute(this);
        DescribeMap dsc;

        dsc["surveyUser"] = query.value("surveyUser").toString();
        dsc["surveyTime"] = query.value("surveyTime").toLongLong();
        dsc["editTime"] = query.value("editTime").toLongLong();
        infoDes->m_time = query.value("routeTime").toLongLong();
        dsc["routeTime"] = infoDes->m_time;
        infoDes->m_bUploaded = query.value("uploaded").toBool();
        infoDes->m_strDescribe = query.value("describe").toString();
        infoDes->m_operationHeight = query.value("operationHeight").toDouble();
        infoDes->ParseInfoString(query.value("routeInfo").toString());
        infoDes->m_totalVoyage = query.value("totalVoyage").toDouble();
        infoDes->m_transVoyage = query.value("transVoyage").toDouble();
        infoDes->m_operateVoyage = query.value("operateVoyage").toDouble();

        QString strOutLine = query.value("routes").toString();
        if (!strOutLine.isEmpty())
        {
            foreach(const QString &str, strOutLine.split("#"))
            {
                VGOutline *tmp = new VGOutline(infoDes, VGLandPolygon::FlyRoute);
                if (initalPolylineByString(*tmp, str))
                    infoDes->m_route = tmp;
                else
                    delete tmp;
                infoDes->_calculateAnggle(tmp);
            }
        }
        QString strSupplys = query.value("supplies").toString();
        if (!strSupplys.isEmpty())
        {
            QStringList strLs = strSupplys.split(",", QString::SkipEmptyParts);
            if (strLs.count() < 3)
                continue;

            QGeoCoordinate coordinate;
            coordinate.setLongitude(strLs.at(0).toDouble());
            coordinate.setLatitude(strLs.at(1).toDouble());
            coordinate.setAltitude(strLs.at(2).toDouble());
        }
        infoDes->m_bSaveLocal = true;
        emit sigQueryItem(infoDes, dsc);
    }
}

bool VGDbManager::_createTableSurvey()
{
    QSqlQuery query(*m_dbase);
    if (!isExistTable("VGLandSurveys"))
    {
        QString strSql = "CREATE TABLE IF NOT EXISTS VGLandSurveys("
			"actualId varchar(32), "
            "surveyUser varchar(50) NOT NULL, "
			"address varchar(100), "
			"surveyTime BIGINT NOT NULL, "
			"surveyPrecision INT NOT NULL DEFAULT 1, "
            "uploaded BOOL, "
            "owner TEXT, "
            "surveyType INTEGER NOT NULL, "
			"plantCoordinates TEXT)";

        if (!query.exec(strSql))
        {
            QString strError = "create Table VGLandSurveys error:" + query.lastError().text();
            qDebug() << strError;
            emit sigSqlResult(DB_TABLEERROR);
            return false;
        }
    }
    return true;
}

bool VGDbManager::_createTableBoundary()
{
    QSqlQuery query(*m_dbase);
    if (!isExistTable("VGLandBuandarys"))
    {
        QString strSql = "CREATE TABLE IF NOT EXISTS VGLandBuandarys("
			"boundaryId varchar(32), "
			"surveyUser varchar(50) NOT NULL, "
			"surveyTime BIGINT NOT NULL, "
			"editTime BIGINT NOT NULL, "
			"uploaded BOOL, "
			"describe varchar(50), "
			"boundary TEXT, "
			"blocksCoordinates TEXT, "
			"safaArea TEXT)";

        if (!query.exec(strSql))
        {
            QString strError = "create Table VGLandBuandarys error:" + query.lastError().text();
            qDebug() << strError;
            emit sigSqlResult(DB_TABLEERROR);
            return false;
        }
    }
    return true;
}

bool VGDbManager::_createTableRoute()
{
    QSqlQuery query(*m_dbase);
    if (!isExistTable("VGFlyRoutes"))
    {
        QString strSql = "CREATE TABLE IF NOT EXISTS VGFlyRoutes("
			"describe varchar(50), "
			"surveyUser varchar(50) NOT NULL, "
			"surveyTime BIGINT NOT NULL, "
            "editTime BIGINT NOT NULL, "
            "routeTime BIGINT NOT NULL, "
			"uploaded BOOL, "
            "operationHeight DOUBLE, "
            "routeInfo TEXT, "
            "totalVoyage DOUBLE,"
            "transVoyage DOUBLE,"
            "operateVoyage DOUBLE,"
			"routes TEXT,"
            "supplies TEXT)";

        if (!query.exec(strSql))
        {
            QString strError = "create Table VGFlyRoutes error:" + query.lastError().text();
            qDebug() << strError;
            emit sigSqlResult(DB_TABLEERROR);
            return false;
        }
    }
    return true;
}

bool VGDbManager::_isExistRecord(const MapAbstractItem &item)
{
    DescribeMap dsc;
    VGGlobalFunc::initialItemDescribe(dsc, item, false);
    VGGlobalFunc::initialTableName(dsc, item.ItemType());

    DescribeMap::const_iterator itr = dsc.find("table");
    if (itr == dsc.end())
        return false;

    QSqlQuery query(*m_dbase);
    QString strSql = QString("select count(*) from %1").arg(itr.value().toString()) + describeMap2Condition(dsc);

    if (query.exec(strSql))
    {
        if (query.next())
            return query.value(0).toInt() > 0;
    }
    return false;
}

bool VGDbManager::createTables()
{
    if(!m_dbase)
        return false;

    if (!_createTableSurvey())
        return false;
    if (!_createTableBoundary())
        return false;
    if (!_createTableRoute())
        return false;

    return true;
}

QString VGDbManager::polygon2String(const VGLandPolygon &lstCoordinate)
{
    QString strOutlines = "";
	foreach (VGCoordinate *itr, lstCoordinate.GetCoordinates())
	{
		if (!strOutlines.isEmpty())
			strOutlines += ";";

		strOutlines += coordinateToString(*itr);
	}
	return strOutlines;
}

QString VGDbManager::polyline2String(const VGOutline &polyLine)
{
    QString strOutlines = "";
    foreach (VGCoordinate *itr, polyLine.GetCoordinates())
    {
        if (!strOutlines.isEmpty())
            strOutlines += ";";

        strOutlines += coordinateToString(*itr);
    }
    return strOutlines;
}

QString VGDbManager::coordinateToString(const VGCoordinate &coor)
{
    QGeoCoordinate co = coor.GetCoordinate();
    QString strCoordinate = QString("%1,%2,%3,%4").arg(QString::number(co.longitude(), 'g', 9)).arg(QString::number(co.latitude(), 'g', 9))
        .arg(QString::number(co.altitude(), 'g', 9)).arg(coor.GetId());

    return strCoordinate;
}

bool VGDbManager::updateTables()
{
    if(!m_dbase)
        return false;

    QSqlQuery query(*m_dbase);

    QString strSql = "ALTER TABLE VGPlantInformations ADD registerId varchar(100)";
    QString strSql2 = "ALTER TABLE VGPlantInformations ADD registerTime INTEGER";
    if(!query.exec(strSql))
        qDebug() << "alter Table is error (VGPlantInformations):" << query.lastError().text();

    if(!query.exec(strSql2))
        qDebug() << "alter Table is error (VGPlantInformations):" << query.lastError().text();

    //升级表 增加服务器上地块和测绘信息id
    strSql = "ALTER TABLE VGPlantInformations ADD actualId varchar(100)";
    query.exec(strSql);
    strSql = "ALTER TABLE VGPlantSurveys ADD boundaryId varchar(100)";
    query.exec(strSql);

    return true;
}

void VGDbManager::onSaveItem(MapAbstractItem *item)
{
    if (!m_bDBOpened)
    {
        emit sigSqlResult(DB_OPENERROR);
        return;
    }

    if (!item || item->IsSaveLocal() || _isExistRecord(*item))
        return;

    if (item->ItemType() == MapAbstractItem::Type_LandInfo)
        _saveLand(*(VGLandInformation*)item);
    if (item->ItemType() == MapAbstractItem::Type_LandBoundary)
        _saveBoundary(*(VGLandBoundary*)item);
    if (item->ItemType() == MapAbstractItem::Type_FlyRoute)
        _saveRoute(*(VGFlyRoute*)item);
}

void VGDbManager::onUpdateItem(const MapAbstractItem *item)
{
    if (!m_bDBOpened)
    {
        emit sigSqlResult(DB_OPENERROR);
        return;
    }

    if (!item || !_isExistRecord(*item))
        return;

    if (item->ItemType() == MapAbstractItem::Type_LandInfo)
        _updateLand(*(VGLandInformation*)item);
    if (item->ItemType() == MapAbstractItem::Type_LandBoundary)
        _updateBoundary(*(VGLandBoundary*)item);
    if (item->ItemType() == MapAbstractItem::Type_FlyRoute)
        _updateRoute(*(VGFlyRoute*)item);
}

void VGDbManager::onDeleteItem(const DescribeMap &dsc)
{
    DescribeMap::const_iterator itr = dsc.find("table");
    if (dsc.find("table") == dsc.end())
        return;

    QString strTable = itr.value().toString();
    if (!isExistTable(strTable))
        return;

    if (!m_bDBOpened || !m_dbase)
    {
        emit sigSqlResult(DB_OPENERROR);
        return;
    }

    QSqlQuery query(*m_dbase);
    QString strSql = QString("delete from %1").arg(strTable) + describeMap2Condition(dsc);
    if (!query.exec(strSql))
        emit sigSqlResult(DB_DELETEERROR);

    return;
}

void VGDbManager::queryItems(const DescribeMap &condition)
{
    DescribeMap::const_iterator itr = condition.find("table");
    if (condition.find("table") == condition.end())
        return;

    QString strTable = itr.value().toString();
    if (!isExistTable(strTable))
        return;

    QString strSql = QString("select * from %1").arg(strTable) + describeMap2Condition(condition);
    QSqlQuery query(*m_dbase);

    if (!query.exec(strSql))
    {
        emit sigSqlResult(DB_QUERYERROR);
        return;
    }

    if (strTable == "VGLandSurveys")
        _processSurveys(query);
    else if (strTable == "VGLandBuandarys")
        _processBoundary(query);
    else if (strTable == "VGFlyRoutes")
        _processRoute(query);
}

bool VGDbManager::initalCoordinateByString(VGCoordinate &coor, const QString &str)
{
    QStringList strLs = str.split(",", QString::SkipEmptyParts);
    if (strLs.count() < 3)
        return false;

    QGeoCoordinate coordinate;
    coordinate.setLongitude(strLs.at(0).toDouble());
    coordinate.setLatitude(strLs.at(1).toDouble());
    coordinate.setAltitude(strLs.at(2).toDouble());

    int sig = strLs.count() >= 4 ? strLs.at(3).toInt() : 0;
    coor.SetCoordinate(coordinate);
    coor.SetId(sig);
    return true;
}

bool VGDbManager::initalPolygonByString(VGLandPolygon &plg, const QString &str)
{
    foreach(const QString &strItr, str.split(";", QString::SkipEmptyParts))
    {
        QStringList strLs = strItr.split(",", QString::SkipEmptyParts);
        if (strLs.count() < 3)
            continue;

        QGeoCoordinate coordinate;
        coordinate.setLongitude(strLs.at(0).toDouble());
        coordinate.setLatitude(strLs.at(1).toDouble());
        coordinate.setAltitude(strLs.at(2).toDouble());

        int sig = strLs.count() >= 4 ? strLs.at(3).toInt() : 0;
        plg.AddCoordinate(coordinate, sig);
    }

    return plg.CountCoordinate()>=3;
}

bool VGDbManager::initalPolylineByString(VGOutline &plg, const QString &str)
{
    QStringList strLsPnt = str.split(";", QString::SkipEmptyParts);
    foreach(const QString &strItr, strLsPnt)
    {
        QStringList strLs = strItr.split(",", QString::SkipEmptyParts);
        if (strLs.count() < 3)
            continue;

        QGeoCoordinate coordinate;
        coordinate.setLongitude(strLs.at(0).toDouble());
        coordinate.setLatitude(strLs.at(1).toDouble());
        coordinate.setAltitude(strLs.at(2).toDouble());

        int sig = strLs.count() >= 4 ? strLs.at(3).toInt() : 0;
        plg.addCoordinate(coordinate, sig);
    }

    return plg.coordinateCount()>0;
}
