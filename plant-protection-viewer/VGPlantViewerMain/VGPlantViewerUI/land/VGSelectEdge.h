#ifndef __VGSELECTEDGE_H__
#define __VGSELECTEDGE_H__

#include "MapAbstractItem.h"
class QGeoCoordinate;
class VGCoordinate;
//地图上显示的轮廓线
class VGSelectEdge : public SingleTriggerItem<MapAbstractItem>
{
    Q_OBJECT

    Q_PROPERTY(QVariantList path READ path NOTIFY pathChanged)
public:
    explicit VGSelectEdge(QObject *parent = 0);
    ~VGSelectEdge();

    QVariantList path()const;
    void SetBeg(const QGeoCoordinate coor);
    void SetEnd(const QGeoCoordinate coor);
    void Show(bool b);
    void SetSelected(bool b);
    bool operator==(const MapAbstractItem &item)const;
    MapItemType ItemType() const;
protected slots:
    void onSelectedChanged(bool b);
    void onGenMid();
signals:
    void  pathChanged(const QVariantList &path);
    void  selected(VGSelectEdge *edge);
    void _toGenMid();
private:
    void _gentSelectPoint(const QGeoCoordinate &coor);
private:
    QVariantList            m_path;         //
    VGCoordinate            *m_coorMid;     //点  
};
 
#endif // VGOUTLINE_H
