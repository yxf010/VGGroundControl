#ifndef VGPLANNINGWORKER_H
#define VGPLANNINGWORKER_H

#include <QObject>
#include <QGeoCoordinate>
#include "RoutePlanning.h"
#include <QVariant>
#include <QList>
/*
 *航线规划线程对象类
 *
*/
class VGFlyRoute;
class VGLandPolygon;
class VGLandBoundary;
typedef QList<QGeoCoordinate> CoordinateList;

class VGPlanningWorker : public QObject, public RoutePlanning
{
    Q_OBJECT
public:
    enum PointPosType {
        OK,
        OutBoundary,
        InBlock,
    };
public:
    explicit VGPlanningWorker(QObject *parent = 0);

    QList<CoordinateList> GenShrinkBoudary(VGFlyRoute *rt);
    bool IsSafeRoute(VGFlyRoute *rt, const QGeoCoordinate &p1, const QGeoCoordinate &p2);
public:
    static QGeoCoordinate GetPolygonCenter(const VGLandPolygon &plg);
    static void TrancePlygon(const QGeoCoordinate &home, const VGLandPolygon &ldPlg, VGPolygon &plg);
    static double CalculatePolygonArea(const VGLandPolygon &ldPlg);
    static PointPosType GetCoordinatePos(const VGLandBoundary &bdr, const QGeoCoordinate &c);
public slots:
    void sltPlanningPlant(VGFlyRoute *rt);
protected:
    void PalnningFinish(const VGRoute &res, bool bSuccess = true);
signals:
    void sigPlanningFinished(const VGRoute &pll);
private:
    bool _genPlanBoudary(VGFlyRoute *rt);
private:
    VGLandBoundary *m_landBoundary;
    QGeoCoordinate m_pntSupport;
};

#endif // VGPLANNINGWORKER_H
