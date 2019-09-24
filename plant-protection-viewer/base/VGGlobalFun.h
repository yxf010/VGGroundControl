#ifndef __VGGLOBALFUN_H__
#define __VGGLOBALFUN_H__

#include <QByteArray>
#include <QPointF>
#include <QGeoCoordinate>
#include <QVariant>
#include "MapAbstractItem.h"
#include "ShareFunction.h"

class MissionItem;
class VGOutline;
class VGCoordinate;

namespace VGGlobalFunc{
    int bytesToInt(const QByteArray &src);
    unsigned short bytesToUShort(const QByteArray &src);
    QByteArray intToBytes(int value);

    QGeoCoordinate gpsCorrect(const QGeoCoordinate &coordinate);
    QGeoCoordinate toGps(const QGeoCoordinate &coordinate);

    //转换火星坐标到百度坐标；
    QPointF trans2BdLatLon(double lon, double lat);
    //经纬度转平面坐标系(米勒坐标投影)
    QPointF coordinate2Miller(double lon, double lat);
    float checkSprink(const VGCoordinate &coCur);
    float checkAngle(const QList<VGCoordinate*> &coors);
    MissionItem *TransItems(const VGCoordinate &coors, float alt, float angle, float p3, float tmRoit = .5, float rRoit = .5);

    void initialTableName(DescribeMap &condition, MapAbstractItem::MapItemType tp);
    bool initialItemDescribe(DescribeMap &condition, const MapAbstractItem &item, bool bId=true);
    DescribeMap ParentDescribe(const DescribeMap &condition, const MapAbstractItem &item);
    QString GetPosTypeTip(int t);
    int GetPosTypeByMavFix(int fix, int  &acc, int &gps);
}

#endif // __VGGLOBALFUN_H__

