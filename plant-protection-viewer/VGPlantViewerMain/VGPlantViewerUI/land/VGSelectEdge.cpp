#include "VGSelectEdge.h"
#include "VGCoordinate.h"

VGSelectEdge::VGSelectEdge(QObject *parent) : SingleTriggerItem<MapAbstractItem>(parent, 0)
, m_coorMid(NULL)
{
    SetItemColor(UnselectedEdgeCol, SelectedEdgeCol);
    connect(this, &VGSelectEdge::_toGenMid, this, &VGSelectEdge::onGenMid);
}

VGSelectEdge::~VGSelectEdge()
{
    delete m_coorMid;
}

QVariantList VGSelectEdge::path() const
{
    return m_path;
}

void VGSelectEdge::SetBeg(const QGeoCoordinate coor)
{
    if (m_path.size() < 2)
        m_path.push_front(QVariant::fromValue(coor));
    else
        m_path.first() = QVariant::fromValue(coor);

    emit _toGenMid();
    emit pathChanged(m_path);
}

void VGSelectEdge::SetEnd(const QGeoCoordinate coor)
{
    if (m_path.size() < 2)
        m_path.push_back(QVariant::fromValue(coor));
    else
        m_path.last() = QVariant::fromValue(coor);

    emit _toGenMid();
    emit pathChanged(m_path);
}

MapAbstractItem::MapItemType VGSelectEdge::ItemType() const
{
    return MapAbstractItem::Type_SelectEdge;
}

void VGSelectEdge::onSelectedChanged(bool b)
{
    if (m_coorMid == sender() && b)
    {
        SetSelected(b);
        emit selected(this);
    }
}

void VGSelectEdge::onGenMid()
{
    if (m_path.size() < 2)
        return;

    QGeoCoordinate coor = VGCoordinate::getMid(m_path.first().value<QGeoCoordinate>(), m_path.last().value<QGeoCoordinate>());
    if (!m_coorMid)
        _gentSelectPoint(coor);
    else
        m_coorMid->SetCoordinate(coor);
}

void VGSelectEdge::_gentSelectPoint(const QGeoCoordinate &coor)
{
    if (m_coorMid)
        return;

    m_coorMid = new VGCoordinate(coor, VGCoordinate::EdgeSelect, this);
    if (m_coorMid)
    {
        Show(GetVisible());
        connect(m_coorMid, &VGCoordinate::selectedChanged, this, &VGSelectEdge::onSelectedChanged);
    }
}

void VGSelectEdge::Show(bool b)
{
    _show(b);
    if(m_coorMid)
        m_coorMid->Show(b);
}

void VGSelectEdge::SetSelected(bool b)
{
    MapAbstractItem::SetSelected(b);
    if (m_coorMid)
        m_coorMid->SetSelected(b);
}

bool VGSelectEdge::operator==(const MapAbstractItem &item)const
{
    if (item.ItemType() != ItemType())
        return false;

    return true;
}
