/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VGCoordinate.h"
#include <QQmlEngine>
#include <qmath.h>
#include "RouteStruct.h"

#define SatelliteGoodNumb 15
#define SatelliteBadNumb 6
#define GoodNumbCol QColor(32,255,16)
#define BadNumbCol QColor(240,32,16)
#define BoundaryCol QColor(101, 139, 255, 160)
#define BlockBoundaryCol QColor(255, 101, 139, 160)
#define BoundarySlelectCol QColor("#FFDF1F")

static QColor GetSatelliteNumbHintColor(int nSat)
{
    if (nSat < SatelliteBadNumb)
        nSat = SatelliteBadNumb;

    if (nSat > SatelliteGoodNumb)
        nSat = SatelliteGoodNumb;

    const double l = (nSat - SatelliteBadNumb)*1.0 / (SatelliteGoodNumb - SatelliteBadNumb);

    QColor ret;
    ret.setRed(GoodNumbCol.red()*l + BadNumbCol.red()*(1-l));
    ret.setGreen(GoodNumbCol.green()*l + BadNumbCol.green()*(1 - l));
    ret.setBlue(GoodNumbCol.blue()*l + BadNumbCol.blue()*(1 - l));
    ret.setAlpha(GoodNumbCol.alpha()*l + BadNumbCol.alpha()*(1 - l));

    return ret;
}
//////////////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////////////
VGCoordinate::VGCoordinate(const QGeoCoordinate& coord, int nSat, QObject* parent)
    : MapAbstractItem(parent, nSat)
    , m_coordinate(coord)
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    m_width = 6;
    onIdChanged(nSat);
    connect(this, &VGCoordinate::idChanged, this, &VGCoordinate::onIdChanged);
}

VGCoordinate::VGCoordinate(const VGCoordinate &oth) : MapAbstractItem(oth)
    , m_coordinate(oth.m_coordinate)
{
}

const QGeoCoordinate &VGCoordinate::GetCoordinate(void) const
{
    return m_coordinate;
}

void VGCoordinate::SetCoordinate(const QGeoCoordinate& coordinate)
{
    if (m_coordinate != coordinate)
    {
        m_coordinate = coordinate;
        emit coordinateChanged(coordinate);
    }
}

void VGCoordinate::SetSatelliteNumbOrType(int nNumb)
{
    if (nNumb >= 0)
        return SetColor(GetSatelliteNumbHintColor(nNumb));

    switch (nNumb)
    {
    case Boundary:
        return SetColor(GetSelected() ? BoundarySlelectCol : BoundaryCol);
    case BlockBoundary:
        return SetColor(GetSelected() ? BoundarySlelectCol : BlockBoundaryCol);
    case RouteOperate:
    case RouteTransfer:
        return SetColor(Qt::white);
    case RouteBegin:
        return SetColor(Qt::green);
    case RouteBack:
        return SetColor(Qt::red);
    case SupportEnter:
        return SetColor(QColor(0,255,0,32));
    case SupportReturn:
        return SetColor(QColor(255, 0, 0, 32));
    case SetSupportEnter:
    case SetSupportReturn:
        return SetColor(QColor(155, 10, 0, 128));
    case EdgeSelect:
        return SetColor(GetSelected() ? SelectedEdgeCol : UnselectedEdgeCol);
    default:
        m_color = Qt::transparent;
        break;
    }
}

MapAbstractItem::MapItemType VGCoordinate::ItemType() const
{
    switch (GetId())
    {
    case Home:
    case HomeSet:
        return Type_Home;
    case Route_Start:
    case Route_End:
        return Type_RouteTip;
    case A_Point:
        return Type_APoint;
    case B_Point:
        return Type_BPoint;
    case SupportEnter:
    case SupportReturn:
    case SetSupportEnter:
    case SetSupportReturn:
        return Type_SupportPoint;
    case ContinueFly:
        return Type_ContinueFly;
    case RouteBegin:
    case RouteOperate:
    case RouteTransfer:
    case RouteBack:
    case RouteLand:
    case RouteCourse:
        return Type_Route;
    case EdgeSelect:
        return Type_PointSelect;
    default:
        return Type_Point;
    }
}

QColor VGCoordinate::GetColor() const
{
    return m_colorBG;
}

void VGCoordinate::SetColor(const QColor &col)
{
    if (m_colorBG == col)
        return;

    m_colorBG = col;
    emit colorChanged(col);
}

void VGCoordinate::_setBDColById(int id)
{
    switch (id)
    {
    case RouteOperate:
        return SetBorderColor(Qt::green);
    case RouteTransfer:
        return SetBorderColor(Qt::red);
    case RouteBegin:
    case RouteBack:
        return SetBorderColor(Qt::white);
    default:
        SetBorderColor(Qt::white);
        break;
    }
}

int VGCoordinate::GetIndex() const
{
    if (m_parentItem)
        return m_parentItem->ItemIndex(this);
    return -1;
}

bool VGCoordinate::IsValid() const
{
    return m_coordinate.isValid();
}

double VGCoordinate::DistanceTo(const VGCoordinate &oth) const
{
    return m_coordinate.distanceTo(oth.m_coordinate);
}


double VGCoordinate::DistanceToCoor(const QGeoCoordinate &c) const
{
    return m_coordinate.distanceTo(c);
}

double VGCoordinate::AzimuthTo(const VGCoordinate &oth) const
{
    return m_coordinate.azimuthTo(oth.m_coordinate);
}

void VGCoordinate::SetAltitude(double altitude)
{
    return m_coordinate.setAltitude(altitude);
}

double VGCoordinate::GetLatitude() const
{
    return m_coordinate.latitude();
}

double VGCoordinate::GetLongitude() const
{
    return m_coordinate.longitude();
}

double VGCoordinate::GetAltitude() const
{
    return m_coordinate.altitude();
}

void VGCoordinate::transCoor2XY(const QGeoCoordinate &home, const QGeoCoordinate &coor, double &x, double &y)
{
    double angle = home.azimuthTo(coor);
    double distance = home.distanceTo(coor);

    x = qSin(2 * M_PI * angle / 360) * distance;
    y = qCos(2 * M_PI * angle / 360) * distance;
}

void VGCoordinate::transXY2Coor(const QGeoCoordinate &home, double x, double y, QGeoCoordinate &coor)
{
    double distance = qSqrt(x*x + y*y);
    double angle = 0;
    double radian = qAtan2(y, x); //(-M_PI, M_PI)

    if (x > 0 && y >= 0)
        angle = M_PI / 2 - radian;
    else if (x > 0 && y < 0)
        angle = M_PI / 2 - radian;
    else if (x < 0 && y < 0)
        angle = M_PI / 2 - radian;
    else if (x < 0 && y >= 0)
        angle = 5 * M_PI / 2 - radian;
    else if (x == 0 && y >= 0)
        angle = 0;
    else if (x == 0 && y < 0)
        angle = M_PI;

    coor = home.atDistanceAndAzimuth(distance, qRadiansToDegrees(angle));
}

QGeoCoordinate VGCoordinate::coordinateFromString(const QString &coor)
{
	QGeoCoordinate ret;
	QStringList lst = coor.split(";", QString::SkipEmptyParts);
	if (lst.count())
		ret.setLongitude(lst.takeFirst().toDouble());
	if (lst.count())
		ret.setLatitude(lst.takeFirst().toDouble());

	return ret;
}

QGeoCoordinate VGCoordinate::getMid(const QGeoCoordinate &c1, const QGeoCoordinate &c2)
{
    return QGeoCoordinate((c1.latitude() + c2.latitude()) / 2,
        (c1.longitude() + c2.longitude()) / 2,
        (c1.altitude() + c2.altitude()) / 2 );
}

QString VGCoordinate::coordinate2String(const QGeoCoordinate &coor)
{
	if (coor.isValid())
		return QString("%1;%2").arg(coor.longitude()).arg(coor.latitude());

	return QString();
}

void VGCoordinate::onIdChanged(int id)
{
    SetSatelliteNumbOrType(id); 
    _setBDColById(id);
    emit colorChanged(GetColor());
    switch (id)
    {
    case RouteBegin:
    case RouteBack:
        return SetWidth(10);
    case RouteOperate:
    case RouteTransfer:
        return SetWidth(7);
    case SupportEnter:
    case SupportReturn:
    case SetSupportEnter:
    case SetSupportReturn:
        return SetWidth(20);
    default:
        break;
    }
}

void VGCoordinate::onSelectedChanged(bool)
{
    SetSatelliteNumbOrType(GetId());
    emit colorChanged(GetColor());
}

void VGCoordinate::adjust(MapAbstractItem::Oriente ori, double val)
{
    if (ori < East || ori>North || DoubleEqual(val, 0) || !IsValid())
        return;

    double x = 0;
    double y = 0;
    switch (ori)
    {
    case East:
        x = val; break;
    case South:
        y = -val; break;
    case West:
        x = -val; break;
    case North:
        y = val; break;
        break;
    default:
        break;
    }
    QGeoCoordinate coor;
    transXY2Coor(m_coordinate, x, y, coor);
    coor.setAltitude(m_coordinate.altitude());
    SetCoordinate(coor);
}

bool VGCoordinate::operator==(const MapAbstractItem &item) const
{
    if (!ItemType() != item.ItemType())
        return false;
    return m_coordinate == ((VGCoordinate*)&item)->m_coordinate;
}
