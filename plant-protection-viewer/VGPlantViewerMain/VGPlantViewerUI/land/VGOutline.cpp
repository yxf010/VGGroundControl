#include "VGOutline.h"
#include <QDebug>
#include "MapItemFactory.h"
#include "VGApplication.h"
#include "VGMapManager.h"
#include "VGLandPolygon.h"

VGOutline::VGOutline(QObject *parent, int id) : MapAbstractItem(parent, id), m_showType(Show_LineAndPoint)
{
    switch (id)
    {
    case VGLandPolygon::Outline:
        SetBorderColor(QColor(255 * 0.4, 255 * 0.9, 255 * 0.4, 255));
        break;
    case VGLandPolygon::Block:
        SetBorderColor(QColor("#af2010"));
        break;
    case VGLandPolygon::Boundary:
    case VGLandPolygon::BlockBoundary:
        SetBorderColor(QColor(255 * 0.4, 255 * 0.4, 255 * 0.9, 255));
        break;
    case VGLandPolygon::SafeBoundary:
        SetBorderColor(QColor(15, 255, 15, 255));
        break;
    case VGLandPolygon::FlyRoute:
        SetBorderColor(QColor("#FFA000"));
        break;
    default:
        SetBorderColor(Qt::green);
        break;
    }
}

VGOutline::VGOutline(const VGOutline &oth) : MapAbstractItem(oth)
, m_showType(oth.m_showType), m_path(oth.m_path)
{
    foreach(VGCoordinate *itr, oth.m_coors)
    {
        _genCoordinate(itr->GetCoordinate(), itr->GetId());
    }
}

VGOutline::~VGOutline()
{
    qDeleteAll(m_coors);
}

void VGOutline::addCoordinate(QGeoCoordinate coordinate, int id)
{
    m_path.append(QVariant::fromValue(coordinate));

    if (GetId()!=VGLandPolygon::NoPoint)
        _genCoordinate(coordinate, id);

    emit pathChanged(m_path);
}

void VGOutline::removeLastCoordinate()
{
    if (!m_path.count())
        return;

    QGeoCoordinate coor = m_path.takeLast().value<QGeoCoordinate>();
    foreach (VGCoordinate *itr, m_coors)
    {
        if (coor == itr->GetCoordinate())
        {
            m_coors.removeOne(itr);
            delete itr;
            break;
        }
    }
    emit pathChanged(m_path);
}

int VGOutline::coordinateCount() const
{
    return m_path.count();
}

QGeoCoordinate VGOutline::coordinateAt(int idx) const
{
    if (idx < 0 || idx >= m_path.count())
        return QGeoCoordinate();

    return m_path.at(idx).value<QGeoCoordinate>();
}

void VGOutline::removeCoordinate(int idx)
{
    if (idx < 0 || idx >= m_path.count())
        return;

    m_path.removeAt(idx);
    emit pathChanged(m_path); 
    if (GetId() != VGLandPolygon::NoPoint)
        m_coors.takeAt(idx)->deleteLater();
}

QGeoCoordinate VGOutline::lastCoordinate() const
{
    if (m_path.count())
        return m_path.last().value<QGeoCoordinate>();

    return QGeoCoordinate();
}

void VGOutline::changeLastCoordinate(const QGeoCoordinate coor, int id)
{
    if (m_path.count() > 0)
    {
        m_path.last() = QVariant::fromValue(coor);
        emit pathChanged(m_path);
        if (GetId() != VGLandPolygon::NoPoint)
            m_coors.last()->SetCoordinate(coor);
    }
    else
    {
        addCoordinate(coor, id);
    }
}

void VGOutline::clear()
{
    if (m_path.count())
    {
        m_path.clear();
        emit pathChanged(m_path);
    }

    qDeleteAll(m_coors);
    m_coors.clear();
}

MapAbstractItem::MapItemType VGOutline::ItemType() const
{
    return MapAbstractItem::Type_PolyLine;
}

void VGOutline::Show(bool b)
{
    if (VGMapManager *mgr = qvgApp->mapManager())
    {
        bool bShowLine = b && (Show_Line&m_showType);
        _show(bShowLine);
        bShowLine ? mgr->InsertItem(this) : mgr->RemoveItem(this);
        QList<MapAbstractItem *> ls;
        for (VGCoordinate *itr : m_coors)
        {
            if (_isCoordinateVisble(*itr))
                ls << itr;
        }
        if (ls.size())
        {
            if (b)
                mgr->InsertItems(ls.first()->ItemType(), ls);
            else
                mgr->RemoveItems(ls.first()->ItemType(), ls);
        }
    }
}

void VGOutline::SetShowType(int t)
{
    if (m_showType == t)
        return;

    m_showType = t;
    Show(GetVisible());
}

const QList<VGCoordinate*> &VGOutline::GetCoordinates() const
{
    return m_coors;
}

int VGOutline::ItemIndex(const MapAbstractItem *item)const
{
    if (!item || item->ItemType() != MapAbstractItem::Type_Point)
        return -1;

    return m_coors.indexOf((VGCoordinate*)item)+1;
}

void VGOutline::ChangeAll(const QList<VGCoordinate*> &coors)
{
    clear();
	m_coors = coors;
    QList<MapAbstractItem*> items;
    MapItemType tp = coors.count()>0?coors.first()->ItemType() : Type_Unknow;
	for (VGCoordinate *c : coors)
	{
		m_path.append(QVariant::fromValue(c->GetCoordinate()));
		if (GetId()!=VGLandPolygon::NoPoint && tp!=Type_Route)
		{
			connect(c, &VGCoordinate::destroyed, this, &VGOutline::onPointDelete);
			connect(c, &VGCoordinate::coordinateChanged, this, &VGOutline::onPointChanged);
        }
        if (c->GetId() != VGCoordinate::RouteCourse)
            items << c;
	}
    VGMapManager *mgr = qvgApp->mapManager();
    if (Type_Unknow!=tp && mgr && GetVisible())
        mgr->InsertItems(tp, items);

	emit pathChanged(m_path);
}

void VGOutline::onPointDelete()
{
	int idx = m_coors.indexOf((VGCoordinate*)sender());
    if (idx >= 0 && idx < m_path.count())
    {
		m_path.removeAt(idx);
		emit pathChanged(m_path);
		m_coors.removeAt(idx);
		QList<VGCoordinate*>::iterator itr = m_coors.begin() + idx;
		for (; itr != m_coors.end(); ++itr)
		{
			emit(*itr)->indexChanged();
		}
    }
}

void VGOutline::onPointChanged(const QGeoCoordinate &co)
{
    VGCoordinate *coor = qobject_cast<VGCoordinate *>(sender());
    if (coor)
    {
        int idx = m_coors.indexOf(coor);
        if (idx >= 0 && idx < m_path.count())
        {
            m_path[idx].setValue(co);
            emit pathChanged(m_path);
        }
    }
}

bool VGOutline::_isCoordinateVisble(const VGCoordinate &coor) const
{
    int nId = coor.GetId();
    return (Show_Point&m_showType) && nId != VGCoordinate::RouteLand && nId != VGCoordinate::RouteCourse;
}

void VGOutline::_genCoordinate(const QGeoCoordinate &coor, int id)
{
    if (VGCoordinate *tmp = new VGCoordinate(coor, id, this))
    {
        m_coors << tmp;
        tmp->Show(GetVisible() && _isCoordinateVisble(*tmp));
        if (tmp->ItemType()!=Type_Route)
        {
            connect(tmp, &VGCoordinate::destroyed, this, &VGOutline::onPointDelete);
            connect(tmp, &VGCoordinate::coordinateChanged, this, &VGOutline::onPointChanged);
        }
    }
}

bool VGOutline::operator==(const MapAbstractItem &item)const
{
    if (item.ItemType() != ItemType())
        return false;

    const VGOutline &ol = *static_cast<const VGOutline*>(&item);

    QVariantList::const_iterator itr = m_path.begin();
    QVariantList::const_iterator itr2 = ol.m_path.begin();
    for (; itr != m_path.end() && itr2 != ol.m_path.end(); ++itr, ++itr2)
    {
        if (itr->value<QGeoCoordinate>() != itr2->value<QGeoCoordinate>())
            return false;
    }
    return true;
}
