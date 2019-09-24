#include "VGGlobalFun.h"
#include <QtMath>
#include <QDebug>
#include <stdint.h>
#include "VGMacro.h"
#include "VGCoordinate.h"
#include "MissionItem.h"
#include "VGLandInformation.h"
#include "VGLandBoundary.h"
#include "VGFlyRoute.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////
//namespace VGGlobalFunc
/////////////////////////////////////////////////////////////////////////////////////////////////////
QByteArray VGGlobalFunc::intToBytes(int value)
{
    QByteArray abyte0(sizeof(value), 0);
    if (ShareFunction::IsBigEndian())
    {
        for (size_t i = 0; i < sizeof(value); ++i)
        {
            abyte0.data()[sizeof(value) - i - 1] = ((char *)&value)[i];
        }
        return abyte0;
    }
    memcpy(abyte0.data(), &value, 4);
    return abyte0;
}

int VGGlobalFunc::bytesToInt(const QByteArray &src)
{
    int value;
    if (ShareFunction::IsBigEndian())
    {
        for (size_t i = 0; i < sizeof(value); ++i)
        {
            ((char *)&value)[i] = src.data()[sizeof(value) - i - 1];
        }
    }
    else
    {
        memcpy(&value, src.data(), sizeof(value));
    }

    return value;
}

unsigned short VGGlobalFunc::bytesToUShort(const QByteArray &src)
{
    unsigned short value;

    if (ShareFunction::IsBigEndian())
    {
        for (size_t i = 0; i < sizeof(value); ++i)
        {
            ((char *)&value)[i] = src.data()[sizeof(value) - i - 1];
        }
    }
    else
    {
        memcpy(&value, src.data(), sizeof(value));
    }
    return value;
}

QGeoCoordinate VGGlobalFunc::gpsCorrect(const QGeoCoordinate &coordinate)
{
    double lat = coordinate.latitude();
    double lon = coordinate.longitude();
    ShareFunction::gpsCorrect(lat, lon);
    return QGeoCoordinate(lat, lon, coordinate.altitude());
}

QGeoCoordinate VGGlobalFunc::toGps(const QGeoCoordinate &coordinate)
{
    double lat = coordinate.latitude();
    double lon = coordinate.longitude();
    ShareFunction::toGps(lat, lon);
    return QGeoCoordinate(lat, lon, coordinate.altitude());
}

QPointF VGGlobalFunc::trans2BdLatLon(double lon, double lat)
{
    double z = qSqrt(lon*lon + lat*lat) + 0.00002*qSin(lat * M_PI * 3000.0 / 180.0);
    double theta = qAtan2(lat, lon) + 0.000003*qCos(lon * M_PI * 3000.0 / 180.0);
    double bdLon = z * qCos(theta) + 0.0065;
    double bdLat = z * qSin(theta) + 0.006;
    QPointF pt;
    pt.setX(bdLon);
    pt.setY(bdLat);

    return pt;
}

QPointF VGGlobalFunc::coordinate2Miller(double lon, double lat)
{
    QPointF pointF;
    double L = 6381372 * M_PI * 2;  //地球周长
    double W = L;                   // 平面展开后，x轴等于周长
    double H = L / 2;                 // y轴约等于周长一半
    double mill = 2.3;              // 米勒投影中的一个常数，范围大约在正负2.3之间
    double x = (lon * M_PI) / 180;  // 将经度从度数转换为弧度
    double y = (lat * M_PI) / 180;  // 将纬度从度数转换为弧度
    y = 1.25 * qLn(qTan(0.25 * M_PI + 0.4 * y));// 米勒投影的转换
                                                // 弧度转为实际距离
    x = (W / 2) + (W / (2 * M_PI)) * x;
    y = (H / 2) - (H / (2 * mill)) * y;

    pointF.setX(x);
    pointF.setY(y);

    return pointF;
}

float VGGlobalFunc::checkSprink(const VGCoordinate &coCur)
{
    int nId = coCur.GetId();
    return (nId == VGCoordinate::RouteOperate || nId == VGCoordinate::RouteBegin) ? 1 : 0;
}

float VGGlobalFunc::checkAngle(const QList<VGCoordinate*> &coors)
{
    int i = 0;
    for (VGCoordinate *coor : coors)
    {
        i++;
        if (i < coors.size())
        {
            if (checkSprink(*coor))
                return coor->AzimuthTo(*coors.at(i));
        }
    }
    return 0;
}

MissionItem *VGGlobalFunc::TransItems(const VGCoordinate &coor, float alt, float angle, float p3, float tmRoit, float rRoit)
{
    MissionItem* item = new MissionItem(0,
        MAV_CMD_NAV_WAYPOINT,                   //航迹点
        MAV_FRAME_GLOBAL_RELATIVE_ALT,          //相对高度
        tmRoit, rRoit, p3, angle,
        coor.GetLatitude(), coor.GetLongitude(), alt,
        true,                                   // autoContinue
        false );

    return item;
}

void VGGlobalFunc::initialTableName(DescribeMap &condition, MapAbstractItem::MapItemType tp)
{
    switch (tp)
    {
    case MapAbstractItem::Type_LandInfo:
        condition["table"] = "VGLandSurveys";
        break;
    case MapAbstractItem::Type_LandBoundary:
        condition["table"] = "VGLandBuandarys";
        break;
    case MapAbstractItem::Type_FlyRoute:
        condition["table"] = "VGFlyRoutes";
        break;
    default:
        break;
    }
}

bool VGGlobalFunc::initialItemDescribe(DescribeMap &condition, const MapAbstractItem &item, bool bId)
{
    switch (item.ItemType())
    {
    case MapAbstractItem::Type_LandInfo:
        {
            QString strId = ((VGLandInformation*)&item)->actualSurveyId();
            if (bId && !strId.isEmpty())
            {
                condition["actualId"] = strId;
            }
            else
            {
                condition["surveyTime"] = ((VGLandInformation*)&item)->surveyTime();
                condition["surveyUser"] = ((VGLandInformation*)&item)->userId();
            }
        }
        return true;
    case MapAbstractItem::Type_LandBoundary:
        {
            QString strId = ((VGLandBoundary*)&item)->GetBoundaryId();
            if (bId && !strId.isEmpty())
                condition["boundaryId"] = strId;
            else
                condition["editTime"] = ((VGLandBoundary*)&item)->GetTime();

            if (MapAbstractItem *itemParent = item.GetParentItem())
                return initialItemDescribe(condition, *itemParent, bId);
        }
        return false;
    case MapAbstractItem::Type_FlyRoute:
        {
            const VGFlyRoute *fr = (VGFlyRoute*)&item;
            QString strId = fr->GetActId();
            if (bId && strId != QString::number(fr->GetTime()))
                condition["routeId"] = strId;
            else
                condition["routeTime"] = fr->GetTime();

            if (MapAbstractItem *itemParent = item.GetParentItem())
                return initialItemDescribe(condition, *itemParent, bId);
        }
        return false;
    }
    return false;
}

DescribeMap VGGlobalFunc::ParentDescribe(const DescribeMap &condition, const MapAbstractItem &item)
{
    DescribeMap ret = condition;
    switch (item.ItemType())
    {
    case MapAbstractItem::Type_LandInfo:
        ret.clear();
        break;
    case MapAbstractItem::Type_LandBoundary:
        ret.remove("editTime");
        ret.remove("boundaryId");
        break;
    case MapAbstractItem::Type_FlyRoute:
        ret.remove("routeTime");
        ret.remove("routeId");
        break;
    default:
        break;
    }

    return ret;
}

QString VGGlobalFunc::GetPosTypeTip(int t)
{
    switch (t)
    {
    case 0:
        return QString::fromStdWString(L"未定位");
    case 1:
        return QString::fromStdWString(L"单点");
    case 2:
        return QString::fromStdWString(L"RTD");
    case 4:
        return QString::fromStdWString(L"固定");
    case 5:
        return QString::fromStdWString(L"浮点");
    case 6:
        return QString::fromStdWString(L"惯导");
    case 7:
        return QString::fromStdWString(L"用户设定");
    }
    return "N/A";
}

int VGGlobalFunc::GetPosTypeByMavFix(int fix, int  &acc, int &gps)
{
    gps = (fix >> 6) & 0x03;
    acc = (fix >> 3) & 0x07;
    int rtk = fix & 0x07;
    switch (rtk)
    {
    case GPS_FIX_TYPE_3D_FIX:
        return 1;
    case GPS_FIX_TYPE_DGPS:
        return 2;
    case GPS_FIX_TYPE_RTK_FLOAT:
        return 5;
    case GPS_FIX_TYPE_RTK_FIXED:
        return 4;
    case GPS_FIX_TYPE_STATIC:
        return 6;
    default:
        break;
    }
    return 0;
}
