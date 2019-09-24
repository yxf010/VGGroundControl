#ifndef VGMAPMANAGER_H
#define VGMAPMANAGER_H

#include <QObject>
#include "VGMapEngineManager.h"
#include <QGeoCoordinate>
#include <QStringListModel>
//#include "VGTreeModel.h"
#include "VGMacro.h"
#include "MapAbstractItem.h"
#include <QMap>

class QTimer;
class VGLandPolygon;
class VGOutline;

class VGMapManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(ManagerObject)

    Q_PROPERTY(int mapTypeID READ mapTypeID WRITE setMapTypeID NOTIFY mapTypeIDChanged)
    //Q_PROPERTY(QGeoCoordinate lastKnownHomePosition READ lastKnownHomePosition  CONSTANT)
    Q_PROPERTY(double         flightMapZoom         MEMBER m_flightMapZoom       NOTIFY flightMapZoomChanged)
    Q_PROPERTY(QGeoCoordinate mapCenter             READ mapCenter WRITE setMapCenter NOTIFY mapCenterChanged)
    Q_PROPERTY(int zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    //Q_PROPERTY(VGTreeModel*     treeModel           READ treeModel CONSTANT)
    Q_PROPERTY(bool showScaleRule READ isShowScaleRule WRITE setShowScaleRule NOTIFY showScaleRuleChanged)
    Q_PROPERTY(double centerLat READ centerLat WRITE setCenterLat NOTIFY centerLatChanged)
    Q_PROPERTY(double centerLon READ centerLon WRITE setCenterLon NOTIFY centerLonChanged)
    Q_PROPERTY(int propLength READ GetPropLength NOTIFY propLengthChanged)
    Q_PROPERTY(int mgrObj READ GetManagerObj WRITE SetManagerObj NOTIFY mgrObjChanged)
public:
    enum ManagerObject {
        None = 0,
        Land = 0x01,
        Mission = 0x02,
        Vehicle = 0x04,
    };
public:
    VGMapManager(QObject *parent = 0);
    ~VGMapManager();
    Q_INVOKABLE void updateMapView(double lon0, double lat0, double lon1, double lat1, int minZoom, int maxZoom, const QString& mapName);
    Q_INVOKABLE QString propertyMapName();

    bool InsertItem(MapAbstractItem *item, int index = -1);
	bool InsertItems(MapAbstractItem::MapItemType tp, const QList<MapAbstractItem *> &items, int index = -1);
    bool RemoveItem(MapAbstractItem *item);
	void RemoveItems(MapAbstractItem::MapItemType tp, const QList<MapAbstractItem *> &items);
    MapAbstractItem *SpecItemAt(MapAbstractItem::MapItemType tp, int index)const;
    void ClearSpecItem(MapAbstractItem::MapItemType tp);
    int CountSpecItem(MapAbstractItem::MapItemType tp)const;
    bool HasItem(MapAbstractItem *item)const;

    void   setMapEngineManager(VGMapEngineManager* mgr);
    void   init();
	bool   isShowScaleRule()const;
	void   setShowScaleRule(bool b);

    void   SetMissionBoundary(const QList<double> &bdy);
    void   loadAllOfflineMaps();
    void   SetItemSelected(MapAbstractItem *item, bool b = true);
    MapAbstractItem *GetSelecedItem(MapAbstractItem::MapItemType tp);
    bool   IsItemSelecte(const MapAbstractItem *item)const;
    double GetPixLength()const;
    QGeoCoordinate mapCenter()const;
    bool IsMissionPage()const;
public slots:
    void    sltQmlCreated(QObject *qmlObject, const QUrl &qmlUrl);
    void    sltTileSetChanged();    //离线地图包数量更新

    void    sltStartDownload(const QString &name, int mapType, int minZoom, int maxZoom);
    void    sltCheckTilesSetDownloadStatus();   //检测下载状态
    void    sltCrazySize();                     //一次下载的图片过多；
    void    sltDeleteOfflineMap(int id);        //删除离线地图包
    void    sltExportOfflineMap(int setID, const QString &dirPath);
    void    sltExportMapResult(const QString &setID, const QString &setName, const QList<QGCCacheTile *> &tiles);
    void    sltDownloadStateChanged(int setID, int type);

    //qml
    void  sltSetMapZoomLevel(int level);
    void  sltSetMapCenter(double lat, double lon);
public:
    static void CalcBoundaryByCoor(QList<double> &ls, const QGeoCoordinate &coordinate); 
    static void CalcBoundaryByCoor(QList<double> &ls, const VGOutline &ol);
protected:
    Q_INVOKABLE void  saveConfig();
    Q_INVOKABLE bool isContainsUav(double lat, double lon);
    Q_INVOKABLE void onZoomLevel();
    Q_INVOKABLE QmlObjectListModel *getSpecItems(int tp);
    //计算比例尺,一个像素长度
    Q_INVOKABLE QString calculatePixLength(const QGeoCoordinate &c1, const QGeoCoordinate &c2, int pix=100);

    int mapTypeID()const;
    void setMapTypeID(int type);

    QString GetMapTypeName()const;
    QString GetStreetMapName()const;

    void setMapCenter(const QGeoCoordinate &coordinate);

    int zoomLevel()const { return m_zoomLevel; }
    void setZoomLevel(int level);
    double centerLat()const;
    void setCenterLat(double lat);
    double centerLon()const;
    void setCenterLon(double lon);
    int GetPropLength()const;
    QString _transString(int len);
    int GetManagerObj()const;
    void SetManagerObj(int n);
signals:
    void    mapTypeIDChanged(int mapTypeID);
    void    mapCenterChanged(const QGeoCoordinate &coor);
    void    finished();
    //从数据库获取已经存在的离线地图包信息
    void    updateExistMap(int id, const QString &name, const QString &mapType, const QString &zooms, long tilesNum, long tilesSize);
    //返回离线地图包下载进度
    void    sigTileSetDownloadStatus(int id, const QString &status, const QString &precent);
    void    flightMapZoomChanged(double flightMapZoom);
    void    sigError(int nErrorType);

    void    sigUpdateMapItems();       //地图飞机数发生变化
    void    sigMapCenterChanged(double lon, double lat);
    void    zoomLevelChanged(int level);
    void    boundaryMissionChanged(double e, double w, double s, double n);
    void    showScaleRuleChanged(bool);
    void    centerLatChanged(double f);
    void    centerLonChanged(double f);
    void    propLengthChanged(int pix);
    void    mgrObjChanged();
protected slots:
    void _checkSelected(QObject *item);
private:
	void _readConfig();
	void _writeConfig();
private:
    VGMapEngineManager *m_GCMapEngineManager;
    int     m_mapTypeID;					//当前地图类型ID
    int     m_zoomLevel;
    double  m_leftLon0;                //当前视图范围内左上角经纬度
    double  m_leftLat0;
    double  m_rightLon1;                //当前视图范围内右下角经纬度
    double  m_rightLat1;
    double  m_centerLat;
    double  m_centerLon;
    double  m_flightMapZoom;
    double  m_pixLength;
    int     m_propLength;
    int     m_mgrObj;

    QGeoCoordinate m_mapCenter;
    QString m_strExportMapPath;   //导出地图的路径；

    QMutex                  m_mutex;
	bool					m_bShowSR;  //
	bool					m_bMapChanged;
    bool                    m_bManagerLand;
    //qml
    QMap<MapAbstractItem::MapItemType, QmlObjectListModel*> m_mapItems;
    QMap<MapAbstractItem::MapItemType, MapAbstractItem*>    m_mapSelectedItem;
};

#endif // VGMAPMANAGER_H
