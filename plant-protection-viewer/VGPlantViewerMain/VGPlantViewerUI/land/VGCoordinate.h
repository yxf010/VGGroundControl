#ifndef __VGCOORDINATE_H__
#define __VGCOORDINATE_H__

#include "MapAbstractItem.h"
#include <QGeoCoordinate>

#define SelectedEdgeCol QColor(32,240,16)
#define UnselectedEdgeCol QColor(240,32,132)
/// This is a QGeoCoordinate within a QObject such that it can be used on a QmlObjectListModel
class VGCoordinate : public MapAbstractItem
{
    Q_OBJECT
    Q_ENUMS(CoordinateType)
    Q_PROPERTY(QGeoCoordinate  coordinate  READ GetCoordinate WRITE SetCoordinate NOTIFY coordinateChanged)
    Q_PROPERTY(int index READ GetIndex NOTIFY indexChanged)
public:
    enum CoordinateType{
        Route_Start = -100,
        Route_End,
        A_Point,
        B_Point,
        Boundary,
        BlockBoundary,
        SupportEnter,       //进出地块辅助点
        SupportReturn,       //进出地块辅助点
        SetSupportEnter,
        SetSupportReturn,
        Sequece,
        Home,
        HomeSet,            //设置home
        EdgeSelect,
        ContinueFly,        //中断点

        RouteBegin = -50,   //进场辅助点
        RouteOperate,       //作业开始
        RouteTransfer,      //转移开始
        RouteBack,          //返航辅助点
        RouteLand,          //降落点
        RouteCourse,        //中间过程点
        RouteEnd,           //结束

        SatelliteNumbBeg = 0,
        SatelliteDraw = 100,
    };
public:
    VGCoordinate(const QGeoCoordinate &coor = QGeoCoordinate(), int nSat = 0, QObject* parent = NULL);//nSat卫星数或者类型
    VGCoordinate(const VGCoordinate &oth);
    const QGeoCoordinate &GetCoordinate(void) const;
    void            SetCoordinate(const QGeoCoordinate& coordinate);
    void            SetSatelliteNumbOrType(int nNumb);
    MapItemType     ItemType()const;
    bool operator==(const MapAbstractItem &item)const;
    int GetIndex()const;
    bool            IsValid()const;
    double          DistanceTo(const VGCoordinate &oth)const;
    double          DistanceToCoor(const QGeoCoordinate &c)const;
    double          AzimuthTo(const VGCoordinate &oth)const;
    void            SetAltitude(double altitude);
    double          GetLatitude()const;
    double          GetLongitude()const;
    double          GetAltitude()const;
    Q_INVOKABLE void adjust(MapAbstractItem::Oriente ori, double val=0.2);
signals:
    void indexChanged();
public:
    static void transCoor2XY(const QGeoCoordinate &home, const QGeoCoordinate &coor, double &x, double &y);
	static void transXY2Coor(const QGeoCoordinate &home, double x, double y, QGeoCoordinate &coor);
	static QString coordinate2String(const QGeoCoordinate &coor);
    static QGeoCoordinate coordinateFromString(const QString &coor);
    static QGeoCoordinate getMid(const QGeoCoordinate &c1, const QGeoCoordinate &c2);
protected slots:
    void onIdChanged(int id);
protected:
    void onSelectedChanged(bool b);
    QColor GetColor()const;
    void  SetColor(const QColor &col);
    void _setBDColById(int id);
signals:
    void coordinateChanged(const QGeoCoordinate &coordinate);
private:
    QGeoCoordinate  m_coordinate;
    QColor          m_colorBG;
};

#endif // __VGCOORDINATE_H__