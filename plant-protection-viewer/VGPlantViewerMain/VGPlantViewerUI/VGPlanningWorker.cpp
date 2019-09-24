#include "VGPlanningWorker.h"
#include <QDebug>
#include <QDateTime>
#include "VGFlyRoute.h"
#include "VGLandBoundary.h"
#include "VGLandPolygon.h"
#include "VGCoordinate.h"
#include "QtMath"

static CoordinateList tranceToGeoPolygon(const QGeoCoordinate &home, const VGPolygon &plg)
{
    CoordinateList ret;
    QGeoCoordinate coor;
    for (const VGPoint &pnt : plg.Points())
    {
        VGCoordinate::transXY2Coor(home, pnt.GetX(), pnt.GetY(), coor);
        ret << coor;
    }
    return ret;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//VGPlanningWorker
//////////////////////////////////////////////////////////////////////////////////////////////////////
VGPlanningWorker::VGPlanningWorker(QObject *parent) : QObject(parent)
, m_landBoundary(NULL)
{
}

QList<CoordinateList> VGPlanningWorker::GenShrinkBoudary(VGFlyRoute *rt)
{
    QList<CoordinateList> ret;
    if (!_genPlanBoudary(rt))
        return ret;

    m_boundary.SetBlockExpandValue(rt->GetBlockSafeDis());
    for (int idx : rt->GetEdgeShinkLs())
    {
        m_boundary.SetEdgeChange(-rt->GetEdgeShink(idx), idx);
    }
    m_boundary.Shrink();
    ret.append(tranceToGeoPolygon(m_pntSupport, m_boundary.ShrinkBoudary()));
    for (const VGPolygon &plg : m_boundary.ExpandBlocks())
    {
        ret.append(tranceToGeoPolygon(m_pntSupport, plg));
    }
    return ret;
}

bool VGPlanningWorker::IsSafeRoute(VGFlyRoute *rt, const QGeoCoordinate &p1, const QGeoCoordinate &p2)
{
    if (!_genPlanBoudary(rt))
        return true;

    m_boundary.SetSpaceValue(rt->GetSprinkleWidth());
    m_boundary.SetBlockExpandValue(rt->GetBlockSafeDis());
    for (int idx : rt->GetEdgeShinkLs())
    {
        m_boundary.SetEdgeChange(-rt->GetEdgeShink(idx), idx);
    }
    m_boundary.Shrink();
    VGPoint pnt1;
    VGCoordinate::transCoor2XY(m_pntSupport, p1, pnt1.X(), pnt1.Y());
    VGPoint pnt2;
    VGCoordinate::transCoor2XY(m_pntSupport, p2, pnt2.X(), pnt2.Y());
    return m_boundary.IsSafeRoute(pnt1, pnt2);
}

QGeoCoordinate VGPlanningWorker::GetPolygonCenter(const VGLandPolygon &plg)
{
    if (plg.CountCoordinate() <= 0)
        return QGeoCoordinate();

    QGeoCoordinate coor = plg.GetCoordinates().first()->GetCoordinate();
    VGPolygon pg; 
    TrancePlygon(coor, plg, pg);
    VGPoint pnt = pg.GetCenter();
    QGeoCoordinate ret;
    VGCoordinate::transXY2Coor(coor, pnt.GetX(), pnt.GetY(), ret);
    return ret;
}

void VGPlanningWorker::sltPlanningPlant(VGFlyRoute *rt)
{
    if (!_genPlanBoudary(rt))
        return;

    m_boundary.SetSpaceValue(rt->GetSprinkleWidth());
    m_boundary.SetBlockExpandValue(rt->GetBlockSafeDis()+.1);
    for (int idx : rt->GetEdgeShinkLs())
    {
        m_boundary.SetEdgeChange(-rt->GetEdgeShink(idx)-.1, idx);
    }
    m_boundary.Shrink();

    m_boundary.SetOrientation(qDegreesToRadians(rt->GetAngle()));
    m_boundary.SetAnti(rt->GetAnti());
    Palnning();
}

void VGPlanningWorker::PalnningFinish(const VGRoute &res, bool bSuccess /*= true*/)
{
    if (bSuccess)
        emit  sigPlanningFinished(res);
}

bool VGPlanningWorker::_genPlanBoudary(VGFlyRoute *rt)
{
    if (!rt)
        return false;

    VGLandBoundary *bdr = rt->GetBelongedBoundary();
    m_pntSupport = rt->SupportPoint();
    bool ret = bdr && m_pntSupport.isValid();
    if (ret && m_landBoundary!=bdr )
    {
        m_landBoundary = bdr;
        m_boundary.ClearBlocks();
        foreach(VGLandPolygon *plg, m_landBoundary->Polygos())
        {
            if (plg->GetId() == VGLandPolygon::Boundary)
            {
                TrancePlygon(m_pntSupport, *plg, m_boundary);
            }
            else if (plg->GetId() == VGLandPolygon::BlockBoundary)
            {
                VGPolygon pg;
                TrancePlygon(m_pntSupport, *plg, pg);
                m_boundary.AddBlock(pg);
            }
        }
    }
    return ret;
}

void VGPlanningWorker::TrancePlygon(const QGeoCoordinate &home, const VGLandPolygon &ldPlg, VGPolygon &plg)
{
    plg.Clear();
    for(const QVariant &var : ldPlg.path())
    {
        double x, y;
        VGCoordinate::transCoor2XY(home, var.value<QGeoCoordinate>(), x, y);
        plg.SetPoint(x, y);
    }
}

double VGPlanningWorker::CalculatePolygonArea(const VGLandPolygon &plg)
{
    VGPolygon plgTmp;
    TrancePlygon(plg.GetCoordinates().first()->GetCoordinate(), plg, plgTmp);
    return plgTmp.CalculatePolygonArea();
}

VGPlanningWorker::PointPosType VGPlanningWorker::GetCoordinatePos(const VGLandBoundary &bdr, const QGeoCoordinate &c)
{
    VGLandPolygon *outline = bdr.GetBoundaryPolygon();
    if (!outline || !outline->IsValid())
        return OutBoundary;

    VGPolygon plgTmp;
    QGeoCoordinate cTmp = outline->GetCoordinates().first()->GetCoordinate();
    TrancePlygon(cTmp, *outline, plgTmp);
    VGPoint p;
    VGCoordinate::transCoor2XY(cTmp, c, p.X(), p.Y());
    if (!plgTmp.IsContains(p))
        return OutBoundary;

    for (VGLandPolygon *itr : bdr.Polygos())
    {
        if(itr ==outline)
            continue;
        plgTmp.Clear();
        TrancePlygon(cTmp, *itr, plgTmp);
        if (plgTmp.IsContains(p))
            return InBlock;
    }

    return OK;
}
