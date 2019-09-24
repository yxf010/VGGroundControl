#ifndef VGMAPVIEWITEM_H
#define VGMAPVIEWITEM_H

#include <QGeoCoordinate>
#include <QTime>
#include "MapAbstractItem.h"
#include "QmlObjectListModel.h"

class VGMapViewItem : public MapAbstractItem
{
    Q_OBJECT

    Q_PROPERTY(QString itemId READ itemId WRITE setItemId NOTIFY itemIdChanged)
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate WRITE setCoordinate NOTIFY coordinateChanged)
    Q_PROPERTY(QGeoCoordinate homePosition READ homePosition WRITE setHomePosition NOTIFY homePositionChanged)
    Q_PROPERTY(QGeoCoordinate lastCoordinate READ lastCoordinate CONSTANT)
    Q_PROPERTY(QmlObjectListModel* trajectoryPoints READ trajectoryPoints CONSTANT)
    Q_PROPERTY(double headingAngle READ headingAngle WRITE setHeadingAngle NOTIFY headingAngleChanged)
    Q_PROPERTY(bool visibleTrajectory READ visibleTrajectory WRITE setVisibleTrajectory NOTIFY visibleTrajectoryChanged)
    Q_PROPERTY(bool selected READ selected WRITE setSelected NOTIFY selectedChanged)
    Q_PROPERTY(bool followed READ followed WRITE setFollowed NOTIFY followedChanged)

public:
    explicit VGMapViewItem(/*int id, */QObject *parent = 0);
    ~VGMapViewItem();


    QString itemId() { return _id; }
    void setItemId(QString id) { _id = id; emit itemIdChanged(id); }

    QGeoCoordinate coordinate() { return _coordinate; }
    void setCoordinate(QGeoCoordinate coordinate);

    QGeoCoordinate lastCoordinate() { return _mapTrajectoryLastCoordinate; }

    QGeoCoordinate homePosition() { return _homePosition; }
    void setHomePosition(QGeoCoordinate homePosition) { _homePosition = homePosition; emit homePositionChanged(homePosition); }

    double headingAngle() { return _headingAngle; }
    void setHeadingAngle(double headingAngle) { _headingAngle = headingAngle; emit headingAngleChanged(headingAngle); }

    bool visibleTrajectory() { return _visibleTrajectory; }
    void setVisibleTrajectory(bool visibleTrajectory) { _visibleTrajectory = visibleTrajectory; emit visibleTrajectoryChanged(visibleTrajectory); }

    QmlObjectListModel* trajectoryPoints(void) { return &_mapTrajectoryList; }

    bool selected() { return _selected; }
    void setSelected(bool selected) { _selected = selected; emit selectedChanged(selected); }

    bool followed() { return _followed; }
    void setFollowed(bool followed) { _followed = followed; emit followedChanged(followed); }


    QTime   startTime() { return _startTime; }
    void    setStartTime(QTime time);

protected:
    MapItemType ItemType()const { return Type_ViewItem; }
    bool operator==(const MapAbstractItem &item)const;
signals:
    void itemIdChanged(QString id);
    void coordinateChanged(QGeoCoordinate coordinate);
    void homePositionChanged(QGeoCoordinate coordinate);
    void headingAngleChanged(double headingAngle);
    void visibleTrajectoryChanged(bool visible);
    void selectedChanged(bool selected);
    void followedChanged(bool followed);

public slots:


private:
    QString         _id;
    bool            _visible;            //显示该item
    bool            _selected;
    bool            _followed;
    QGeoCoordinate  _homePosition;       //初始点坐标
    QGeoCoordinate  _coordinate;         //当前坐标
    double          _headingAngle;       //方向角度0为正北方
    bool            _visibleTrajectory;  //绘制轨迹
    QmlObjectListModel  _mapTrajectoryList;  //轨迹
    QGeoCoordinate  _mapTrajectoryLastCoordinate;    //轨迹上一个点坐标

    QTime           _startTime;
};

#endif // VGMAPVIEWITEM_H
