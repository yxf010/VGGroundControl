#include "VGLandPolygon.h"
#include "MapItemFactory.h"
#include "VGCoordinate.h"
#include "VGApplication.h"
#include "VGMapManager.h"
#include "VGPlanningWorker.h"

VGLandPolygon::VGLandPolygon(QObject *parent, int tp) : MapAbstractItem(parent, tp)
, m_typeShow(Show_LineAndPoint)
{
    connect(this, &VGLandPolygon::idChanged, this, &VGLandPolygon::onIdChange);
    onIdChange(tp);
}

VGLandPolygon::VGLandPolygon(VGLandPolygon &polygon) : MapAbstractItem(polygon)
, m_path(polygon.m_path), m_typeShow(polygon.m_typeShow)
{
    connect(this, &VGLandPolygon::idChanged, this, &VGLandPolygon::onIdChange);
    for (VGCoordinate *itr : polygon.m_coors)
    {
        VGCoordinate *tmp = _getCoordinate(itr->GetCoordinate(), itr->GetId());
        m_coors << tmp;
        tmp->Show(GetVisible() && (m_typeShow & Show_Point));
    }
}

VGLandPolygon::~VGLandPolygon()
{
    qDeleteAll(m_coors);
}

QVariantList VGLandPolygon::path() const
{
    return m_path;
}

void VGLandPolygon::setPath(const QVariantList &path, const QList<VGCoordinate *> &coors)
{
    foreach(const QVariant &itr, path)
    {
        if (itr.value<QVariant>().isValid())
            m_path.append(itr);
    }
    emit pathChanged(m_path);
    if (GetId() == NoPoint)
        return;

    foreach(VGCoordinate *itr, coors)
    {
        if (VGCoordinate *tmp = _getCoordinate(itr->GetCoordinate(), itr->GetId()))
        {
            m_coors.append(tmp);
            tmp->Show(GetVisible() && (m_typeShow & Show_Point));
        }
    }
    emit countCoorChanged(m_path.count());
}

void VGLandPolygon::AddCoordinate(const QGeoCoordinate &coor, int nSat, VGCoordinate *c)
{
    if (coor.isValid())
    {
        if (VGCoordinate *item = _getCoordinate(coor, nSat))
        {
            bool bVisible = GetVisible();
            if (m_path.count() == 2)
                _show(false);

            _insertCoor(item, c);
            if (bVisible)
                _show(true);
            item->Show(GetVisible() && (m_typeShow & Show_Point));
            emit pathChanged(m_path);
            emit countCoorChanged(m_path.count());
        }
    }
}

void VGLandPolygon::AddCoordinate(const VGCoordinate &coor)
{
    AddCoordinate(coor.GetCoordinate(), coor.GetId());
}

QList <VGCoordinate *> VGLandPolygon::GetCoordinates() const
{
    return m_coors;
}

int VGLandPolygon::CountCoordinate() const
{
    return m_path.count();
}

double VGLandPolygon::CalculateArea() const
{
    if (m_path.count() < 3)
        return 0;
                                                                                                              
    return VGPlanningWorker::CalculatePolygonArea(*this);
}

MapAbstractItem::MapItemType VGLandPolygon::ItemType() const
{
    return m_path.count() > 2 ? Type_Polygon : Type_PolyLine;
}

void VGLandPolygon::Show(bool b)
{
    if (VGMapManager *mgr = qvgApp->mapManager())
    {
        bool bShowLine = b && (m_typeShow&Show_Line);
        _show(bShowLine);
        bool bShowPoint = b && (m_typeShow&Show_Point);
        foreach(VGCoordinate *itr, m_coors)
        {
            bShowPoint ? mgr->InsertItem(itr) : mgr->RemoveItem(itr);
        }
    }
}

void VGLandPolygon::SetShowType(int t)
{
    if (m_typeShow == t)
        return;

    m_typeShow = t;
    Show(GetVisible());
}

int VGLandPolygon::ItemIndex(const MapAbstractItem *item) const
{
    if (!item || item->ItemType() != MapAbstractItem::Type_Point)
        return -1;

    return m_coors.indexOf((VGCoordinate*)item)+1;
}

bool VGLandPolygon::IsValid() const
{
    return m_path.count() > 2;
}

void VGLandPolygon::Clear()
{
    bool bVisi = GetVisible();
    if (bVisi)
        Show(false);
    while (m_coors.count())
    {
        m_coors.takeFirst()->deleteLater();
    }
    m_path.clear();
    Show(bVisi);
    emit pathChanged(m_path);
}

bool VGLandPolygon::operator==(const MapAbstractItem &item) const
{
    if (item.ItemType() != ItemType() || GetId()!=item.GetId())
        return false;

    const VGLandPolygon &ol = *static_cast<const VGLandPolygon*>(&item);
    if (m_path.count() != ol.m_path.count())
        return false;

    QVariantList::const_iterator itr = m_path.begin();
    QVariantList::const_iterator itr2 = ol.m_path.begin();
    for (; itr != m_path.end() && itr2 != ol.m_path.end(); ++itr, ++itr2)
    {
        if (itr->value<QGeoCoordinate>() != itr2->value<QGeoCoordinate>())
            return false;
    }
    return true;
}

void VGLandPolygon::onIdChange(int id)
{
    if (GetColor() == QColor(255, 0, 0, 255))
        return;

    switch (id)
    {
    case Outline:
        SetBorderColor(QColor(255 * 0.9, 255 * 0.4, 255 * 0.4, 255));
        break;
    case Block:
        SetBorderColor(QColor("#af2010"));
        break;
    case Boundary:
        SetBorderColor(QColor(255 * 0.4, 255 * 0.4, 255 * 0.9, 255));
        break;
    case BlockBoundary:
        SetBorderColor(QColor("#FF00A0"));
        break;
    case SafeBoundary:
        SetBorderColor(QColor(15, 255, 15, 255));
        break;
    default:
        SetBorderColor(Qt::green);
        break;
    }
}

void VGLandPolygon::onPointDelete()
{
    VGCoordinate *coor = (VGCoordinate*)sender();
	int idx = m_coors.indexOf(coor);
	if (idx >= 0)
	{
		m_coors.removeAt(idx);
		QList<VGCoordinate*>::iterator itr = m_coors.begin() + idx;
		for (; itr != m_coors.end(); ++itr)
		{
			emit(*itr)->indexChanged();
        }

        bool bVisible = GetVisible();
        if (m_coors.count() == 2)
        {
            _show(false);
            SetVisible(bVisible);
        }

        m_path.removeAt(idx);
        if (bVisible && (m_typeShow & Show_Line))
            _show(true);

        emit pathChanged(m_path);
        emit countCoorChanged(m_coors.size());
	}
}

void VGLandPolygon::onPointChanged(const QGeoCoordinate &co)
{
    VGCoordinate *coor = qobject_cast<VGCoordinate *>(sender());
    if (coor)
    {
        int idx = m_coors.indexOf(coor);
        if (idx >= 0)
        {
            m_path[idx].setValue(co);
            emit pathChanged(m_path);
        }
    }
}

VGCoordinate *VGLandPolygon::_getCoordinate(const QGeoCoordinate &co, int id)
{
    VGCoordinate *tmp = new VGCoordinate(co, id, this);
    if (tmp)
    {
        tmp->SetVisible(true);
        connect(tmp, &VGCoordinate::destroyed, this, &VGLandPolygon::onPointDelete);
        connect(tmp, &VGCoordinate::coordinateChanged, this, &VGLandPolygon::onPointChanged);
    }
    return tmp;
}

void VGLandPolygon::_insertCoor(VGCoordinate *item, VGCoordinate *behind)
{
    if (!item)
        return;

    int index = behind ? m_coors.indexOf(behind) : -1;
    if (index < 0)
        m_coors << item;
    else
        m_coors.insert(index+1, item);


    if (index < 0)
    {
        m_path.append(QVariant::fromValue(item->GetCoordinate()));
        return;
    }
    else
    {
        m_path.insert(index + 1, QVariant::fromValue(item->GetCoordinate()));
        QList<VGCoordinate*>::iterator itr = m_coors.begin() + index+1;
        for (; itr != m_coors.end(); ++itr)
        {
            emit(*itr)->indexChanged();
        }
    }
    emit countCoorChanged(m_coors.size());
}
