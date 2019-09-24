#include "VGSupportPolyline.h"

VGSupportPolyline::VGSupportPolyline(QObject *parent)
    :MapAbstractItem(parent), m_bValid(true)
{
    SetWidth(1);
}

VGSupportPolyline::VGSupportPolyline(const VGSupportPolyline &oth) :MapAbstractItem(oth)
, m_path(oth.m_path), m_bValid(oth.m_bValid)
{
}

void VGSupportPolyline::SetBegin(const QGeoCoordinate &co)
{
    if(m_path.count()>=2)
    {
        if (co == m_path.first().value<QGeoCoordinate>())
            return;
        m_path.first().setValue(co);
    }
    else
    {
        m_path.push_front(QVariant::fromValue<QGeoCoordinate>(co));
    }
    emit pathChanged(m_path);
}

void VGSupportPolyline::SetEnd(const QGeoCoordinate &co)
{
    if (m_path.count() >= 2)
    {
        if (co == m_path.back().value<QGeoCoordinate>())
            return;

        m_path.back().setValue(co);
    }
    else
    {
        m_path.push_front(QVariant::fromValue<QGeoCoordinate>(co));
    }

    emit pathChanged(m_path);
}

void VGSupportPolyline::AddPoint(const QGeoCoordinate &co)
{
    m_path.append(QVariant::fromValue<QGeoCoordinate>(co));
    emit pathChanged(m_path);
}

QVariantList VGSupportPolyline::path() const
{
    return m_path;
}

MapAbstractItem::MapItemType VGSupportPolyline::ItemType() const
{
    return MapAbstractItem::Type_DashLine;
}

void VGSupportPolyline::SetValid(bool b)
{
    if (m_bValid == b)
        return;

    m_bValid = b;
    Show(b && GetVisible());
}

bool VGSupportPolyline::IsValide() const
{
    return m_bValid;
}

int VGSupportPolyline::CountPoint() const
{
    return m_path.count();
}

void VGSupportPolyline::Clear()
{
    if(m_path.count() > 0)
    {
        m_path.clear();
        emit pathChanged(m_path);
    }
}

bool VGSupportPolyline::operator==(const MapAbstractItem &oth) const
{
    if (oth.ItemType() != Type_DashLine)
        return false;

    return m_path == (*(VGSupportPolyline*)&oth).m_path;
}