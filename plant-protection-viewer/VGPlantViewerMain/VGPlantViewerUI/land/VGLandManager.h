#ifndef VGLANDBLOCKOBJECT_H
#define VGLANDBLOCKOBJECT_H

#include "VGLandInformation.h"

class VGGetGpsLocation;
class VGLandInformation;
class VGLandBoundary;
class VGFlyRoute;
class VGBDLocation;
class VGNetManager;
class VGPlanningWorker;

class VGLandManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QGeoCoordinate curCoordinate READ GetCurCoordinate NOTIFY curCoordinateChanged)
    Q_PROPERTY(bool curCoordValid READ IsCurCoordValid NOTIFY curCoordValidChanged)
    Q_PROPERTY(bool isGPSOpened READ isGPSOpened CONSTANT)
    Q_PROPERTY(int landCount READ landCount CONSTANT)
    Q_PROPERTY(int numbSatlate READ GetNumbSatlate NOTIFY numbSatlateChanged)
    Q_PROPERTY(VGLandInformation* curLand READ GetCurLand NOTIFY curLandChanged)
    Q_PROPERTY(VGFlyRoute* curFlyRoute READ GetCurFlyRoute NOTIFY curFlyRouteChanged)
public:
    explicit VGLandManager(QObject *parent = 0);
    ~VGLandManager();

    bool    isGPSOpened() const;
    void    setError(const QString &error);
    void    SetCurCoordinat(const QGeoCoordinate &coor);
    VGBDLocation *GetBDLocation()const;

    Q_INVOKABLE void    startGetGps(int timeOut);       //启动GPS定位功能
    Q_INVOKABLE void    stopGetGps();                   //停止定位功能

    int landCount() const; //我的地块中地块数
    QString GetUserId()const;
    void uploadLandInfo(MapAbstractItem::MapItemType tp, const MapListItem *parent=NULL);
    void SetCurFlyRoute(VGFlyRoute *rt);
    QList<VGFlyRoute*> GetRoutesOfBoundary(VGLandBoundary *bdr);
    void  BoundaryChanged(const QList<double> &bdr);
    int   Indexof(VGFlyRoute *rt);
    void  AddChild(MapAbstractItem *item);
    void  InitialNetWork(VGNetManager *nm);
    VGLandInformation *Clone(VGLandInformation *item, bool bNew=false);
    VGLandBoundary *Clone(VGLandBoundary *item, VGLandInformation *p);
    VGFlyRoute *Clone(VGFlyRoute *item, VGLandBoundary *p);
    VGLandInformation *GetSurveyLand()const;
    QGeoCoordinate GetCurCoordinate() const;
    int GetNumbSatlate()const;

    const QList<VGLandInformation*> &Lands()const;
    const QList<VGLandBoundary*> &Boundaries()const;
    const QList<VGFlyRoute*> &FlyRoutes()const;
public:
    static void CloseGPS();
    static VGGetGpsLocation *GetGps();
    static void PlanRouteFinish(VGFlyRoute *fr);
    static void PlanRoute(VGFlyRoute *fr);
    static VGPlanningWorker *PlanningWorker();
protected:
    //只允许Qml调用
    Q_INVOKABLE  VGFlyRoute *preparePlanRoute(VGLandBoundary *bdy);
    Q_INVOKABLE VGLandInformation *surveyLand();
    Q_INVOKABLE VGLandBoundary *editBoundary(MapAbstractItem *item);
    Q_INVOKABLE void quitOperation();
    VGFlyRoute *GetCurFlyRoute()const;
    VGLandInformation *GetCurLand()const;
    void SetCurLand(VGLandInformation *land); 
    bool IsCurCoordValid()const;

    //overload
    void timerEvent(QTimerEvent *e);
signals:
    void    curCoordinateChanged(const QGeoCoordinate &coor);
    void    errorChanged(const QString &strError);
    void    boundaryCoordinateChanged(double e, double w, double s, double n);
    void    curFlyRouteChanged(VGFlyRoute *rt);
    void    curLandChanged(VGLandInformation *land);
    void    numbSatlateChanged(int numb);
    void    curCoordValidChanged(bool b);
public slots:
    void    onQueryItem(MapAbstractItem *item, const DescribeMap &dsc);
    void    sltSurveyError(int errorInfo);
protected slots :
    void    onChildDestroyed();
    void    onSelectedChanged(bool b);
    void    onLandSurveyed(VGLandInformation *land);//地块测绘完成
    void    onEditFinished(VGLandBoundary *bd);
    void    onGpsPosition(double lat, double lon, double alt, int sat);
    void    onPlanRouteFinished();
private:
    VGLandInformation   *_getLandInformation();
    VGLandBoundary      *_getBoundary();
    MapAbstractItem     *findItem(const DescribeMap &dsc)const;
    VGFlyRoute          *_getPlanFlyRoute(VGLandBoundary *bdy);
    void                initialGPS(VGLandManager *mgr);
    bool                appedSurveyLand(VGLandInformation *land, bool b=true);
    bool				appendBoundary(VGLandBoundary *bdy);
    bool				appendRoute(VGFlyRoute *fr);
private:
    int                                 m_tmId;
	int                                 m_gptTimeout;       //gps超时时间
	int                                 m_connectState;
    int                                 m_numbSatlate;      //卫星数
    QGeoCoordinate                      m_coorCur;
    qint64                              m_tmLastGps;

    //
    QList<VGLandInformation*>           m_lstLand;          //地块列表
    QList<VGLandBoundary*>              m_lstBoundary;      //地边列表
    QList<VGFlyRoute*>                  m_lstRoute;         //规划列表
    VGLandInformation                   *m_landCur;         //当前地块
    VGLandInformation                   *m_landSurvey;      //当前测绘地块
    VGFlyRoute                          *m_routeCur;        //当前航线规划
    VGFlyRoute                          *m_flyRoutePlan;

    VGLandBoundary                      *m_boundaryEdit;    //编辑地边图
    VGBDLocation                        *m_location;
};

#endif // VGLANDBLOCKOBJECT_H
