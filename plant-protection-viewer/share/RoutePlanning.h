#ifndef __ROUTEPLANNING_H__
#define __ROUTEPLANNING_H__

#include "RouteStruct.h"

class VGBoudary : public VGPolygon
{
public:
    SHARED_DECL VGBoudary(const VGPolygon &outline = VGPolygon(), const vector<VGPolygon> &blocks = vector<VGPolygon>());
    SHARED_DECL VGBoudary(const VGBoudary &oth);

    SHARED_DECL void SetOutLine(const VGPolygon &outline);
    SHARED_DECL void AddBlock(const VGPolygon &plg);
    SHARED_DECL void SetOrientation(double radius);
    SHARED_DECL void SetSpaceValue(double f);
    SHARED_DECL void ShrinkWidthValue(double f);
    SHARED_DECL void ShrinkLongValue(double f);
    SHARED_DECL void SetBlockExpandValue(double f);
    SHARED_DECL void Shrink();
    SHARED_DECL void SetSupportPoint(const VGPoint &pnt);
    SHARED_DECL void PlanRoute(RoutePlanning *pl);
    SHARED_DECL vector<VGPolygon> &ExpandBlocks();
    SHARED_DECL const VGPolygon &ShrinkBoudary()const;
    SHARED_DECL void ClearBlocks();
    SHARED_DECL void SetAnti(bool b);
    SHARED_DECL void SetEdgeChange(double sh, int idx=-1);
    SHARED_DECL bool GenerateSafeRoute(VGPolyLine &rt, const VGPoint &pnt1, const VGPoint &pnt2)const; //ret:false,pnt1,pnt2线段在界限内
    SHARED_DECL bool IsSafeRoute(const VGPoint &pnt1, const VGPoint &pnt2)const;
private:
    bool _generateRout(VGPolyLine &rt, vector<vector<VGLineSeg>> &segs)const;//beg = 线段的index*2，如果是线段结尾+1
    int  _getSafeRoute(const VGPoint &pnt1, const VGPoint &pnt2, VGPolyLine &rt, VGPoint *last)const;
    bool _linkRoute(VGPolyLine &rt, vector<vector<VGLineSeg>> &segs, int &nLine, int &nIdx)const;
    int _getNearestSeg(const vector<vector<VGLineSeg>> &segs, int &nLine, const VGPoint &pnt, bool bBeg, VGPolyLine &rt)const;
    void _blockDistance(map<double, const VGPolygon*> &blocks, const VGLineSeg &seg)const;
    void _blockExpand();
    double _getSafeRouteLength(const VGPoint &pnt1, const VGPoint &pnt2)const;
    double _generateOpSeg(vector<vector<VGLineSeg>> &segs);
    double _calculate1LineSegs(const VGLine &line, vector<VGLineSeg> &rt)const;
    vector<int> _calcSearchs(const vector<vector<VGLineSeg>> &segs,
        const VGPoint &pnt, int nLineCur, bool &bBeg, int &nLine, VGPolyLine &rt)const;
    int _checkOtherLine(const vector<vector<VGLineSeg>> &segs, const vector<int> &serchs, int &nLine, VGPolyLine &rt)const;
    void _removeChangeEdge(int idx);
    VGPoint _getPropertyCorner(const VGPolyLine &rt, const VGPoint &end, const vector<VGPoint> &pnts)const;
private:
    vector<VGPolygon>   m_blocks;
    vector<EdgeChange>  m_edgeChanges;
    VGVertex            m_orin;
    VGPoint             m_pntSupport;
    double              m_space;
    double              m_widthShrink;
    double              m_longShrink;
    double              m_blockExpand;
    bool                m_bBlockValid;
    bool                m_bAnti;
    vector<VGPolygon>   m_expBlocks;
    VGPolygon           m_outline;
};

class RoutePlanning
{
public:
    SHARED_DECL virtual ~RoutePlanning(){}
    SHARED_DECL VGBoudary &Boudary();
    SHARED_DECL void Palnning();
    SHARED_DECL virtual void PalnningFinish(const VGRoute &res, bool bSuccess = true) = 0;
protected:
    VGBoudary   m_boundary;
};

#endif // ROUTEPLANNING_H
