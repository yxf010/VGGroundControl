#ifndef VGDBMANAGER_H
#define VGDBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include "VGMacro.h"
#include "VGPlantInformation.h"

class VGLandInformation;
class VGLandBoundary;
class VGLandPolygon;
class VGCoordinate;
class VGOutline;
class VGFlyRoute;

class VGDbManager : public QObject
{
    Q_OBJECT
public:
    explicit VGDbManager(QObject *parent = 0);
    bool    isDBOpened();
    void    CreateDb(const QString  &dbName);
    void    SaveItemSafe(MapAbstractItem *item);
    void    DeleteItemSafe(MapAbstractItem *item);
    void    QueryItems(MapAbstractItem::MapItemType tp, const MapAbstractItem *parent = NULL);
    void    UpdateDb(const MapAbstractItem *item);
signals:
    void    sigSqlResult(int result);                                            //操作结果  1， 表示成功， 其它值表示失败；（线程中返回结果
    //查询结果通知
    void    sigQueryItem(MapAbstractItem *item, const DescribeMap &parentDsc = DescribeMap());
public:
    static QString toString(const QVariant &var);
    static const QString &getDBOperateDescribe(int res);
private slots:
    bool    createDb(const QString &dbName);
    void    onSaveItem(MapAbstractItem *item);
    void    onUpdateItem(const MapAbstractItem *item);
    void    onDeleteItem(const DescribeMap &dsc);
    void    queryItems(const DescribeMap &condition);                  //查詢用户地地块测绘信息
private:
    bool    createTables();
    bool    updateTables();  //在数据库表结构升级后，兼容老版本数据库，自动添加或者删除表新列

    QString polygon2String(const VGLandPolygon &lstCoordinate);  //轮廓点转换成字符串
    QString polyline2String(const VGOutline &polyLine);  //轮廓点转换成字符串
    QString coordinateToString(const VGCoordinate &coor);
    bool    isExistTable(QString tableName);        //判断数据库某表是否存在

    bool _createTableSurvey();
    bool _createTableBoundary();
    bool _createTableRoute();
    bool _isExistRecord(const MapAbstractItem &item);

    void _saveLand(VGLandInformation &land);
    void _saveBoundary(VGLandBoundary &boundary);
    void _saveRoute(VGFlyRoute &route);
    void _updateLand(VGLandInformation &land);
    void _updateBoundary(VGLandBoundary &boundary);
    void _updateRoute(VGFlyRoute &route);
    void _processSurveys(QSqlQuery &query);
    void _processBoundary(QSqlQuery &query);
    void _processRoute(QSqlQuery &query);
private:
    static bool initalCoordinateByString(VGCoordinate &coor, const QString &str);
    static bool initalPolygonByString(VGLandPolygon &plg, const QString &str);
    static bool initalPolylineByString(VGOutline &plg, const QString &str);
signals:
    void _saveItem(MapAbstractItem *item);
    void _deleteItem(const DescribeMap &dsc);
    void _sigQueryInfo(const DescribeMap &cd);
    void _sigCreatDb(const QString &, bool updateTable = false);
    void _update(const MapAbstractItem *item);
private:
    QSqlDatabase                *m_dbase;
    QString                     m_dbBaseName;
    bool                        m_bDBOpened;
};
#endif // VGDBMANAGER_H
