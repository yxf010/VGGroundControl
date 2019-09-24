#ifndef VGOUTLINE_H
#define VGOUTLINE_H

#include "MapAbstractItem.h"
#include <QVariant>
#include "VGCoordinate.h"

//地图上显示的轮廓线
class VGOutline : public MapAbstractItem
{
    Q_OBJECT

    Q_PROPERTY(QVariantList path READ path NOTIFY pathChanged)
public:
    explicit VGOutline(QObject *parent = 0, int id = 0);
    explicit VGOutline(const VGOutline &oth);
    ~VGOutline();

    QVariantList path()const { return m_path; }

    void addCoordinate(QGeoCoordinate coordinate, int id=VGCoordinate::Boundary);
    void removeLastCoordinate();
    int coordinateCount()const;
    QGeoCoordinate coordinateAt(int idx)const;
    void removeCoordinate(int idx);
    QGeoCoordinate lastCoordinate()const;
    void changeLastCoordinate(const QGeoCoordinate coor, int id=0);

    void    clear();
    MapItemType ItemType()const;
    bool operator==(const MapAbstractItem &item)const;

    void Show(bool b);
    void SetShowType(int t);
    const QList<VGCoordinate*> &GetCoordinates()const;
    int ItemIndex(const MapAbstractItem *item)const;
    void ChangeAll(const QList<VGCoordinate*> &ls);
signals:
    void    pathChanged(const QVariantList &path);
protected slots:
    void onPointDelete();
    void onPointChanged(const QGeoCoordinate &coor);
private:
    bool _isCoordinateVisble(const VGCoordinate &coor)const;
    void _genCoordinate(const QGeoCoordinate &coor, int id);
private:
    QVariantList            m_path;     //
    QList<VGCoordinate*>    m_coors;    //点
    int                     m_showType; //      
};
 
#endif // VGOUTLINE_H
