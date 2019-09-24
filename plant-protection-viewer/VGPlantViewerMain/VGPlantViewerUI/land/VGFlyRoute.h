#ifndef __VGFLYPLAN_H__
#define __VGFLYPLAN_H__

#include "MapAbstractItem.h"

class VGLandBoundary;
class VGLandInformation;
class VGOneRoute;
class VGCoordinate;
class QGeoCoordinate;
class VGDbManager;
class VGNetManager;
class VGOutline;
class VGRoute;
class VGVehicleMission;
class MissionItem;
class VGSelectEdge;

class VGFlyRoute : public SingleTriggerItem<MapAbstractItem>
{
    Q_OBJECT
    Q_PROPERTY(bool isPlanning READ isPlanning NOTIFY planningChanged)
    Q_PROPERTY(float sprinkleWidth READ GetSprinkleWidth WRITE SetSprinkleWidth NOTIFY sprinkleWidthChanged)
    Q_PROPERTY(float sprinkleCover READ GetSprinkleCover WRITE SetSprinkleCover NOTIFY sprinkleCoverChanged)
    Q_PROPERTY(float operationHeight READ GetOperationHeight WRITE SetOperationHeight NOTIFY operationHeightChanged)
    Q_PROPERTY(double maxRoute READ GetMaxRoute WRITE SetMaxRoute NOTIFY maxRouteChanged)
    Q_PROPERTY(double blockSafe READ GetBlockSafeDis WRITE SetBlockSafeDis NOTIFY blockSafeChanged)
    Q_PROPERTY(double outlineSafe READ GetOutlineSafe WRITE SetOutlineSafe NOTIFY outlineSafeChanged)
    Q_PROPERTY(float angle READ GetAngle WRITE SetAngle NOTIFY angleChanged)
    Q_PROPERTY(double supportHeight READ GetSupportHeight WRITE SetSupportHeight NOTIFY supportHeightChanged)
    Q_PROPERTY(double totalVoyage READ GetTotalVoyage CONSTANT)
    Q_PROPERTY(double transVoyage READ GetTransVoyage CONSTANT)
    Q_PROPERTY(double operateVoyage READ GetOperateVoyage CONSTANT)
    Q_PROPERTY(double area READ GetLandArea CONSTANT)
    Q_PROPERTY(qint64 planTime READ GetTime NOTIFY planTimeChanged)
    Q_PROPERTY(QString landName READ GetLandName NOTIFY landNameChanged)
    Q_PROPERTY(QString user READ GetUser CONSTANT)
    Q_PROPERTY(QString describe READ GetDescribe WRITE SetDescribe NOTIFY describeChanged)
    Q_PROPERTY(QString actId READ GetActId WRITE SetActId NOTIFY actIdchanged)
    Q_PROPERTY(bool isValide READ IsValide NOTIFY valideChanged)
    Q_PROPERTY(bool canPlanRoute READ canPlanRoute NOTIFY canPlanChanged)
    Q_PROPERTY(QString cropper READ GetCropper WRITE SetCropper NOTIFY cropperChanged)
    Q_PROPERTY(QString pesticide READ GetPesticide WRITE SetPesticide NOTIFY pesticideChanged)
    Q_PROPERTY(double price READ GetPrice WRITE SetPrice NOTIFY priceChanged)
    Q_PROPERTY(bool anti READ GetAnti WRITE SetAnti NOTIFY antiChanged)
    Q_PROPERTY(bool singleShrink READ GetSingleShrink WRITE SetSingleShrink NOTIFY singleShrinkChanged)
    Q_PROPERTY(int curEdge READ GetCurEdge NOTIFY curEdgeChanged)
public:
    typedef QPair<int, double> EdgeShrinkPair;
public:
    explicit VGFlyRoute(QObject *info = NULL);
    VGFlyRoute(const VGFlyRoute &oth);
    ~VGFlyRoute();

    VGLandBoundary *GetBelongedBoundary()const;
    VGLandInformation *GetBelongedLand()const;

    MapItemType ItemType()const;
    bool operator==(const MapAbstractItem &item)const;
    qint64 GetTime()const;
    void showContent(bool b);

    bool IsValide()const;
    bool isPlanning()const;
    bool canPlanRoute()const;
    MapAbstractItem *Clone(QObject *parent)const;
    QGeoCoordinate SupportPoint()const;
    float  GetSprinkleWidth()const;
    double GetMaxRoute()const;
    double GetLandArea()const;
    QString GetUser()const;
    float GetAngle()const;

    QString GetInfoString()const;
    void    ParseInfoString(const QString &str);
    double  GetBlockSafeDis()const;
    double  GetOutlineSafe()const;
    void    releaseSafe();
    void    SetSelected(bool b);
    VGOutline *allMissionRoute()const;
    QString GetBoundaryID()const;
    QString GetActId()const;
    void SetActId(const QString &id);
    bool save(bool bCloud);
    Q_INVOKABLE void setBelongBoundary(VGLandBoundary *bdr);
    Q_INVOKABLE VGVehicleMission *beginOperation();
    VGVehicleMission *GetVehicleMission()const;
    QList<MissionItem*> GetMissionItems()const;
    const QList<MissionItem*> &GetSafeBoudaryItems();
    float GetOperationHeight()const;
    void SetOperationHeight(float f);
    float GetMedPerAcre()const;
    void SetMedPerAcre(float f);

    double GetTotalVoyage()const;
    double GetTransVoyage()const;
    double GetOperateVoyage()const;
    void DetachVm(VGVehicleMission *);
    bool GetAnti()const;
    MapAbstractItem *FindParentByProperty(const DescribeMap &dsc)const;
    bool IsSaveLocal() const;
    bool GetSingleShrink()const;
    double GetEdgeShink(int idx)const;
    QList<int> GetEdgeShinkLs()const;
    int CountBlock()const;
public slots:
    void showPlanningResult(const VGRoute &result);
protected:
    void SetSprinkleWidth(float f);
    float GetSprinkleCover()const;
    void SetSprinkleCover(double f);
    void SetMaxRoute(double f);
    QString GetDescribe()const;
    void SetDescribe(const QString &des);
    QString GetLandName()const;

	int GetVoyageNum()const;
    int CountBdrBlocks()const;

	void SetSaveLocal(bool bSl);
	bool IsUpLoaded()const;
    void SetUploaded(bool bUpload);

    QString GetCropper()const;
    void SetCropper(const QString &c);
    QString GetPesticide()const;
    void SetPesticide(const QString &c);
    double GetPrice()const;
    void SetPrice(double f);
    void SetBlockSafeDis(double f);
    void SetOutlineSafe(double f);
    void SetAngle(float f);
    double GetSupportHeight()const;
    void SetSupportHeight(double f);
    void processSaveReslt(const DescribeMap &result);
    void SetAnti(bool b);
    void SetSingleShrink(bool b);
    int GetCurEdge()const;
signals:
    void sigPlanRoute(VGFlyRoute *rt);
    void planRouteFinished();
    void planningChanged(bool p);
    void sprinkleWidthChanged(float f);
    void sprinkleCoverChanged(float f);
    void operationHeightChanged(float f);
    void maxRouteChanged(double f);
    void describeChanged(const QString &des);
    void landNameChanged(const QString &name);
    void valideChanged(bool b);
    void canPlanChanged(bool b);
    void editSpplyChanged(bool b);
    void planTimeChanged(qint64 tm);
    void cropperChanged(const QString &c);
    void pesticideChanged(const QString &p);
    void priceChanged(double p); 
    void blockSafeChanged(double f);
    void outlineSafeChanged(double f);
    void angleChanged(float f);
    void supportHeightChanged(double f);
    void actIdchanged(const QString &name);
    void antiChanged(bool b);
    void singleShrinkChanged(bool b);
    void curEdgeChanged(int idx);
protected slots:
    void onChildDestroied(QObject *obj);
    void onSingleShrinkChanged(bool b);
    void onSelectedEdge(VGSelectEdge *edge);
protected:
    void planRoute(bool chgBdr);
    Q_INVOKABLE void showSelectEdge(bool b);
    QList<double>GetBoundarys()const;
private:
    void _genarateDefualtHome(VGLandBoundary *bdr);
    void _adjustSupportAttitude(double alt);
    void _adjustOperateAttitude(double alt);
    void _calculateAnggle(VGOutline *rt);
    void _calculateSafeBoundary();
    QString _bdrSafeDis2Str()const;
    void _parseSafeDis(const QString &str);
private:
    friend                  VGDbManager;
    friend                  VGNetManager;
    float                   m_sprinkleWidth;                    //喷幅(米）
    float                   m_sprinkleCover;                    //喷幅重叠比例(0~1）    
    float                   m_operationHeight;                  //作业高度（米）
    float                   m_medPerAcre;                       //母用药量
    float                   m_fAngle;                           //边界安全距离
    double                  m_fBlockSafeDis;                    //障碍物安全距离

    double                  m_maxRoute;                         //最大航程（米）
    double                  m_totalVoyage;                      //总航程 公里
    double                  m_transVoyage;                      //转移航程 公里
    double                  m_operateVoyage;                    //作业航程 公里
    qint64                  m_time;
    QString                 m_strDescribe;                      //描述
    QString                 m_strCropper;                       //作物
    QString                 m_strPesticide;                     //药剂
    QString                 m_strActId;                         //服务器ID
    double                  m_fPrise;                           //单价
    VGCoordinate            *m_homePoint;                       //home点
    VGOutline               *m_route;                           //规划
    bool                    m_bSingleShrink;                    //单边
    bool                    m_bPlan;                            //正在规划
    bool                    m_bUploaded;                        //航线是否上传过
    bool                    m_bSaveLocal;
    bool                    m_bErrorPlan;                       //规划错误
    bool                    m_bAnti;                            //从右到左规划
    VGVehicleMission        *m_olMission;                       //任务
    int                     m_nCurEdge;
    QList<VGSelectEdge*>    m_lsSelectEdge;                     //可选边
    double                  m_fSafeDis;                         //障碍物安全距离
    QList<EdgeShrinkPair>   m_lsOutlineSafe;                    //边界安全距离
    QList<MissionItem*>     m_safeBdrs;                         //作业边界安全
};

#endif // __VGFLYPLAN_H__
