#include "VGMapViewItem.h"

VGMapViewItem::VGMapViewItem(/*int id, */QObject *parent) : MapAbstractItem(parent)
{
    //_id = id;
    _followed = false;
    _selected = false;
}

VGMapViewItem::~VGMapViewItem()
{
    _mapTrajectoryList.clear();
}

void VGMapViewItem::setCoordinate(QGeoCoordinate coordinate)
{
    _coordinate = coordinate;
    emit coordinateChanged(coordinate);

    _mapTrajectoryLastCoordinate = coordinate;
}

void VGMapViewItem::setStartTime(QTime time)
{
    _startTime = time;
}

bool VGMapViewItem::operator==(const MapAbstractItem &) const
{
    return false;
}
