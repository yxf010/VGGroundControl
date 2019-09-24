#ifndef __R_STRUCT_H__
#define __R_STRUCT_H__

#include <vector>
#include <map>
#include <string.h>
#include "share_global.h"

#define inf 0x3f3f3f3f
#define PI 3.14159265358979323846
#define  DoubleEqual(x, y) ((int)(((x) - (y)) * 0x00100000) == 0)
#define  DoubleLess(x, y) ((int)(((x) - (y)) * 0x00100000) < 0)

using namespace std;
class VGVertex;
class VGPolyLine;
class VGPolygon;
class RoutePlanning;
class VGLine;

class SHARED_DECL VGPoint
{
public:
    VGPoint(double x=0, double y=0, double z=0);
    VGPoint(const VGPoint &oth);
    virtual ~VGPoint(){}
    double &X();
    double &Y();
    double &Z();
    double GetX()const;
    double GetY()const;
    double GetZ()const;
    double DistanceTo(const VGPoint &oth)const;
    double DistanceSqureTo(const VGPoint &oth)const;
    VGPoint operator-(const VGPoint &oth)const;
    VGPoint &operator-=(const VGPoint &oth);
    VGPoint operator+(const VGPoint &oth)const;
    VGPoint &operator+=(const VGPoint &oth);
    VGPoint operator-(const VGVertex &oth)const;
    VGPoint &operator-=(const VGVertex &oth);
    VGPoint operator+(const VGVertex &oth)const;
    VGPoint &operator+=(const VGVertex &oth);
    VGPoint operator*(double f)const;
    VGPoint &operator*=(double f);
    VGPoint operator/(double f)const;
    VGPoint &operator/=(double f);
    bool operator==(const VGPoint &oth)const;
    bool operator!=(const VGPoint &oth)const;
private:
    double m_x;
    double m_y;
    double m_z;
};

class SHARED_DECL VGVertex  //向量
{
public:
    VGVertex(double x=0, double y=0, double z=0);
    VGVertex(bool b2d, double angleY, double angleXY = 0);
    VGVertex(const VGPoint &beg, const VGPoint &end);
    VGVertex(const VGVertex &oth);

    double &X();
    double &Y();
    double &Z();
    double GetX()const;
    double GetY()const;
    double GetZ()const;
    VGVertex operator-(const VGVertex &oth)const;
    VGVertex &operator-=(const VGVertex &oth);
    VGVertex operator+(const VGVertex &oth)const;
    VGVertex &operator+=(const VGVertex &oth);
    VGVertex operator*(double f)const;
    VGVertex &operator*=(double f);
    VGVertex operator/(double f)const;
    VGVertex &operator/=(double f);
    VGVertex CrossMultiply(const VGVertex &v = VGVertex(0.0, 0, 1))const;
    double  DotMultiply(const VGVertex &v)const;
    bool    IsValid()const;
    bool    operator==(const VGVertex &oth)const;
    bool    operator!=(const VGVertex &oth)const;
    double  UnitLength()const;
    double  UnitLengthSquare()const;
	VGVertex UnitVertex()const;
	double CrossX(const VGVertex &v = VGVertex(0.0, 0, 1))const;
	double CrossY(const VGVertex &v = VGVertex(0.0, 0, 1))const;
	double CrossZ(const VGVertex &v = VGVertex(0.0, 0, 1))const;
private:
    double m_x;
    double m_y;
    double m_z;
};

class SHARED_DECL VGRoutePoint : public VGPoint
{
public:
    enum RouteType
    {
        Enter = 0,
        Operation,
        Transform,
        Return,
        Land,
        Route,
        Unreach,
    };
public:
    VGRoutePoint(double x = 0, double y = 0, double z = 0, int tp = 0);
    VGRoutePoint(const VGPoint &pnt, int tp = 0);
    VGRoutePoint(const VGRoutePoint &pnt);
    void SetType(int t);
    int GetType()const;
private:
    int    m_type;//0-起飞点，1-作业开始，2-转移开始，3-返航点，4-降落点，5-中间过程点
};

class VGSinAngle //(--360)
{
public:
    VGSinAngle(const VGPoint &pnt1, const VGPoint &pnt2, const VGPoint &pnt3);
    VGSinAngle(const VGVertex &v1, const VGVertex &v2);
    VGSinAngle(const VGLine &line, const VGPoint &pnt);
    VGSinAngle();

    double GetSinAngle()const;
    bool   IsAcute()const;
    double GetCos()const;
    bool   IsZero()const;

    bool        operator==(const VGSinAngle &oth)const;
    bool        operator!=(const VGSinAngle &oth)const;
    bool        operator<(const VGSinAngle &oth)const;
    bool        operator>(const VGSinAngle &oth)const;
    VGSinAngle  operator+(const VGSinAngle &oth)const;
    VGSinAngle  operator-(const VGSinAngle &oth)const; 
    VGSinAngle  TranceTriAngle()const;
private:
    double  _calculateSin(const VGLine &line, const VGPoint &pnt);
    double  _calculateCos(const VGLine &line, const VGPoint &pnt);
private:
    double  m_sin;
    double  m_cos;
};

class  VGSinAngleArea
{
public:
    VGSinAngleArea(const VGSinAngle &an = VGSinAngle());
    bool IsValid()const;
    bool Contains(const VGSinAngle &an)const;
    int AdjustByContains(const VGSinAngle &an);
    const VGSinAngle &GetBegin()const;
    const VGSinAngle &GetEnd()const;
private:
    VGSinAngle   m_angleBeg;
    VGSinAngle   m_angleEnd;
};

class VGLineSeg
{
public:
    VGLineSeg(const VGPoint &beg = VGPoint(), const VGPoint &end = VGPoint());
    VGLineSeg(const VGLineSeg &oth);
    double Length()const;
    double LengthSqure()const;
    VGPoint NearestPoint(const VGPoint &point)const; //线段上离point最近点
    double NearestDistance(const VGPoint &point)const; //线段上离point最近点
    VGVertex GetVertex();
    VGPoint GetBegin()const;
    VGPoint GetEnd()const;
    VGPoint GetMid()const;
    VGVertex GetVertex()const;
    bool IsValid()const;
    VGLineSeg Expand(const VGPoint &point, double dis)const;
    VGPoint GetPoint(double t)const;
    double GetParam(const VGPoint &point)const;
    bool IsOnSeg(const VGPoint &point)const;
    VGLine GetRectLineFrom(const VGPoint &pnt, const VGVertex &vC= VGVertex(0.0,0,1.0))const;
private:
    VGPoint m_beg;
    VGPoint m_end;
};

class VGLine
{
public:
    VGLine(const VGPoint &beg, const VGPoint &end);
    VGLine(const VGVertex &v, const VGPoint &pnt=VGPoint());
    VGLine(const VGLineSeg &v);
    VGLine(const VGLine &oth);
    VGLine();
    VGPoint GetOnePoint()const;
    void SetOnePoint(const VGPoint &p);
    VGVertex &Vertex();
    VGVertex GetVertex()const;
    void SetVertex(const VGVertex &v);
    vector<VGPoint> IntersectionsXY(const VGLineSeg &seg)const;//z=0投影交点
    vector<VGPoint> IntersectionsXY(const VGLine &oth)const;//z=0投影交点
    map<double, VGPoint> IntersectionsXY(const VGPolygon &plg)const;
    double DistanceToPoint(const VGPoint &pnt)const;
    VGLine GetRectLineFrom(const VGPoint &pnt)const;
    VGPoint GetDistancePoint(const VGPoint &pnt, double dis)const;
    VGPoint RectPoint(const VGPoint &point)const;//垂直点
    VGPoint GetPoint(double t)const;
    double GetParam(const VGPoint &point)const;
    bool   IsOnLine(const VGPoint &poin)const;
    bool   operator==(const VGLine &oth)const;
    bool   operator!=(const VGLine &oth)const;
private:
    VGPoint     m_piont;
    VGVertex    m_vertex;
};

class VGPolyLine
{
public:
    VGPolyLine(const vector<VGRoutePoint> &points = vector<VGRoutePoint>());
    VGPolyLine(const VGPolyLine &oth);
    virtual ~VGPolyLine(){}

    double Length()const;
    VGLineSeg LineSegAt(int idx)const;
    int CountLineSeg()const;
    int CountPoint()const;
    void AddFrontPoint(const VGPoint &pnt, int type = VGRoutePoint::Route);
    void AddRoutePoint(const VGPoint &pnt, int type = VGRoutePoint::Route);
    bool CalculateNearerWay(const VGPoint &pnt, int type = VGRoutePoint::Route);
    bool CutBeging(const VGPoint &pnt, int type = VGRoutePoint::Route);
    void Clear();
    const VGPoint &First()const;
    const VGPoint &Last()const;
    VGPolyLine Revert()const;
    void RemoveAt(int i);
    const vector<VGRoutePoint> &RoutePoints()const;
protected:
    void _setPoint(int idx, const VGPoint &pnt, int type = 5);
private:
    vector<VGRoutePoint> m_points;
};

class VGRoute :public VGPolyLine
{
public:
    SHARED_DECL VGRoute(const vector<VGRoutePoint> &points = vector<VGRoutePoint>());
    SHARED_DECL VGRoute(const VGRoute &oth);
    SHARED_DECL double &OperateVoyage();
    SHARED_DECL double TotalVoyage()const;
    SHARED_DECL double TransVoyage()const;
    SHARED_DECL double GetOperateVoyage()const;
    SHARED_DECL const vector<VGRoutePoint> &GetRoutePoints()const;
protected:
    double m_operateVoyage;
};

class VGPolygon
{
protected:
    typedef struct EdgeChange {
        int index;
        double edgeShrink;
        EdgeChange(int idx, double p) :index(idx), edgeShrink(p) {}
    }EdgeChange;
public:
    SHARED_DECL VGPolygon();
    SHARED_DECL VGPolygon(const VGPolygon &oth);
    SHARED_DECL virtual ~VGPolygon(){}
    SHARED_DECL void SetPoint(double x, double y, double z=0, int idx=-1);
    SHARED_DECL void Clear();
    SHARED_DECL VGPoint GetCenter(bool bAverage=true)const;//bAverage, true平均位置, false 重心
    SHARED_DECL double CalculatePolygonArea()const;
    SHARED_DECL int CountLineSeg()const;
    SHARED_DECL const vector<VGPoint> &Points()const;
    SHARED_DECL bool IsContains(const VGPoint &pnt, bool bOnBoard = true)const;

    double Length()const;
    VGLineSeg LineSegAt(int idx)const;
    bool IsValid()const;
    VGPolygon GetExpand(double f)const;
    VGPolygon GetExpand(const vector<EdgeChange> &edge)const;
    void SetPoint(const VGPoint &pnt, int idx=-1);
    void Remove(int idx);
    bool IsContains(const VGLineSeg &seg)const;
    bool IsContains(const VGPolyLine &pl, bool bContainsOnBoard = true)const;
    void GetTipPoint(const VGVertex &v, VGPoint &pntMin, VGPoint &pntMax);
    double NearestDistance(const VGPoint &pnt)const;
    double Distance2LineSeg(const VGLineSeg &seg)const;//ret>0, 不相交
    vector<VGPoint> GetCornerPoint(const VGPoint &pnt1, const VGPoint &pnt2, const VGPolygon &outLine=VGPolygon())const;
    bool IsIntersect(const VGLine &line, bool bSkipByBord = true)const;
    bool IsSegOut(const VGLineSeg &se)const;
    bool RoundRoute(const VGPoint &pnt1, const VGPoint &pnt2, VGPolyLine &rt)const;
    void CutSeg(const VGLineSeg &seg, vector<VGLineSeg> &segs)const;//线段裁剪
protected:
    bool _findRoundPnt(const VGPoint &pnt1, const VGPoint &pnt2, const VGPolygon &outLine, VGPoint &pnt)const;
    int _getOnBoarder(const VGPoint &pnt)const;
    int _getLongestLineSegIndex()const;
    bool _getRoundRoute(const VGPoint &pnt1, const VGPoint &pnt2, VGPolyLine &rt1, VGPolyLine &rt2)const;
protected:
    vector<VGPoint> m_points;
};

#endif //
