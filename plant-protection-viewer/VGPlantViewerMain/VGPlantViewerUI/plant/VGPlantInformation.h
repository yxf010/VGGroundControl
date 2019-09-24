#ifndef __VGMYPLANEINFORMATION_H__
#define __VGMYPLANEINFORMATION_H__

#include <QObject>
#include <QGeoCoordinate>
#include <QDateTime>
#include <MapAbstractItem.h>
class VGVehicle;
class VGVehicleMission;
class VGCoordinate;
class VGOutline;
class VGLandInformation;
class QVariant;
class MissionItem;
class VGSupportPolyline;

class VGPlantInformation : public MapAbstractItem
{
    Q_OBJECT
    Q_ENUMS(PlantStatus)

    Q_PROPERTY(VGVehicleMission *vm READ GetVehicleMission NOTIFY missionChanged)
    Q_PROPERTY(QString flightMode READ GetFlightMode WRITE SetFlightMode NOTIFY flightModeChanged)
    Q_PROPERTY(bool arm READ IsSetArm WRITE SetArm NOTIFY armChanged)

    Q_PROPERTY(QString planeId READ planeId NOTIFY planeIdChanged)
    Q_PROPERTY(QString productType READ productType NOTIFY productTypeChanged)
    Q_PROPERTY(QString organization READ organization NOTIFY organizationChanged)
    Q_PROPERTY(QString manager READ manager NOTIFY managerChanged)
    Q_PROPERTY(QString contact READ contact NOTIFY contactChanged)
    Q_PROPERTY(float voltage READ GetVoltage NOTIFY voltageChanged)

    Q_PROPERTY(PlantStatus status READ GetStatus NOTIFY statusChanged)
    Q_PROPERTY(bool binded READ IsBinded NOTIFY bindedChanged)
    Q_PROPERTY(QString master READ master  NOTIFY masterChanged)
    Q_PROPERTY(QString userId READ userId NOTIFY userIdChanged)
    Q_PROPERTY(QString posTip READ GetPosTip NOTIFY posTipChanged)
    Q_PROPERTY(bool directLink READ isDirectLink WRITE setDirectLink NOTIFY directLinkChanged)
    Q_PROPERTY(bool landed READ IsLanded NOTIFY landedChanged)
    Q_PROPERTY(double compassAngle READ GetCompassAngle  NOTIFY compassAngleChanged)
    Q_PROPERTY(double rollAngle READ GetRollAngle  NOTIFY rollAngleChanged)
    Q_PROPERTY(double pitchAngle READ GetPitchAngle  NOTIFY pitchAngleChanged)
    Q_PROPERTY(double horSpeed READ GetSpeedHor NOTIFY horSpeedChanged)
    Q_PROPERTY(double powPercent READ GetPowerPercent NOTIFY powPercentChanged)
    Q_PROPERTY(double medSpeed READ GetMedicineSpeed NOTIFY medSpeedChanged)
    Q_PROPERTY(double medVol READ GetMedicineVol NOTIFY medVolChanged)
    Q_PROPERTY(double precision READ GetPrecision NOTIFY precisionChanged)
    Q_PROPERTY(QGeoCoordinate coordinate READ lastCoordinate NOTIFY coordinateChanged)
    Q_PROPERTY(double heigth READ GetHeigth NOTIFY heigthChanged)
    Q_PROPERTY(int satlateNumb READ GetSatlateNumb NOTIFY satlateNumbChanged)
    Q_PROPERTY(bool monitor READ IsMonitor NOTIFY monitorChanged)
    Q_PROPERTY(bool missionValible READ IsMissionValible NOTIFY missionValibleChanged)
    Q_PROPERTY(QString accTip READ GetAccTip NOTIFY accTipChanged)
    Q_PROPERTY(QString gpsTip READ GetGpsTip NOTIFY gpsTipChanged)
    Q_PROPERTY(bool wlcLose READ IsWlcLose NOTIFY wlcLoseChanged)
    Q_PROPERTY(QString qxacc READ GetQXAcc NOTIFY qxaccChanged)
    Q_PROPERTY(QString qxpswd READ GetQXPswd NOTIFY qxpswdChanged)
public:
    enum PlantStatus
    {
        UnConnect,
        Connected,
    };
public:
    explicit VGPlantInformation(QObject *parent = 0);    
    VGPlantInformation(const VGPlantInformation &oth);
    ~VGPlantInformation();

    QString planeId()const;
    void setPlaneId(const QString &id);
    QString productType()const;
    void setProductType(const QString &id);
    QString organization()const;
    void setOrganization(const QString &id);
    QString manager()const { return m_manager; }
    void setManager(const QString &id);
    QString contact()const { return m_contact; }
    void setContact(const QString &id);

    PlantStatus GetStatus()const;
    void SetStatus(PlantStatus status);

    bool IsBinded()const;
    void SetBinded(bool status);

    QString master()const { return m_master; }
    void setMaster(const QString &master);

    qint64 lastTime()const;
    void setLastTime(qint64 time);
    void setLastConnected();

    qint64 bindTime()const { return m_bindTime; }
    void setBindTime(qint64 time) { m_bindTime = time;}

    qint64 unBindTime()const { return m_unBindTime; }
    void setUnBindTime(qint64 time) { m_unBindTime = time;}

    QString userId()const { return m_userId; }
    void setUserId(const QString &id);

    bool isDirectLink()const;
    void setDirectLink(bool isDirectLink);

    bool IsLanded()const;
    void SetLanded(bool isLanded);

    void setCoordinate(const QGeoCoordinate &coor);
    QGeoCoordinate lastCoordinate()const;
    double distance()const;
    void setDistance(double distance);
    double GetHeigth();
    void SetRelativeAltitude(double f);
    void SetAltitude(double f);
    int GetSatlateNumb()const;
    void SetSatlateNumb(int n);
    double GetSpeedHor()const;
    void SetSpeedHor(double f);
    double GetSpeedVer()const;
    void SetSpeedVer(double f);
    double GetCompassAngle()const;
    void SetCompassAngle(double f);
    double GetRollAngle()const;
    void SetRollAngle(double f);
    double GetPitchAngle()const;
    void SetPitchAngle(double f);
    double GetPowerPercent()const;
    void SetPowerPercent(double f, bool alarm = false);
    double GetMedicineSpeed()const;
    void SetMedicineSpeed(double f);
    double GetMedicineVol()const;
    void SetMedicineVol(double f);
    double GetPrecision()const;
    void SetPrecision(double f);
    void SetMonitor(bool b, bool bCur=false);
    void CheckConnect(qint64 curMs);
    void syncPlanFlight(VGVehicleMission *vm);
    bool operator==(const MapAbstractItem &item)const;
    MapItemType ItemType()const;

    void CommandRes(unsigned short cmd, bool res);
    QString GetFlightMode()const;
    void ChangeFlightMode(const QString &mod);
    void SetFlightMode(const QString &mod);
    void SetMissionValible(bool b, int tp=0);
    void SetCurrentExecuteItem(int idx);
    void SetLastExecuteItem(int idx);
    void SetFixType(unsigned char fixType);
    void LoadFromVehicle();
    void TestPump();
	void TestMotor(int idx, int ms, int speedd);
    void Shutdown();
    void SetPlantParammeter(const QString &key, const QVariant &v);
    void GetPlantParammeter(const QString &key);
    void SychFinish(bool suc);
    void Disarm();
    void SetBaseMode(unsigned short bm);
    void SetSelected(bool);
    void SetSychBegin();
	void SetMedicineGrade(int t);
	void SetPowerGrade(int t);
    bool DealPlantParameter(const QString &key, const QVariant &v);
    void SetGndHeight(double height);
    QVariant GetParamValue(const QString &key);
    bool IsImitateGround();
    void UploadBoundary();
    void SendBoundaryItem(int seq);
    void SetVoltage(uint16_t mv);
    float GetVoltage()const;
    void SendSupport(bool bEnter, VGCoordinate *spt);
    void SetSysStatus(uint8_t st);
    bool IsWlcLose()const;
    void SetMissionCount(int c, bool finish);
    void SetMissionItem(const MissionItem &item, bool finish);
    void SetDownMisson(bool b);
    void SendSetHome(const QGeoCoordinate &c=QGeoCoordinate());
    void SetHome(const QGeoCoordinate &c, bool bRcv = false);
    void ReceiveSupports(const QGeoCoordinate &beg, const QGeoCoordinate &end, const QGeoCoordinate &cut, int st);
    void SetQxAccount(const QString &acc, const QString &pswd, bool bSend=true);
    void GetQxAccount();
    void ClearQxAccount(bool bSend=true);
public:
    static QString GetFlightModeDscb(const QString &str);
public slots:
    void onMissionChange();
protected:
    bool IsMonitor()const;
    bool IsSetArm()const;
    void SetArm(bool b);
    bool IsMissionValible()const;
    QString GetPosTip()const;
    QString GetAccTip()const;
    QString GetGpsTip()const;
    void   showContent(bool b);
    void _arm(bool bArm);
    void _sysFlightMode();
    void SetPosType(int t);
    void SetAccType(int t);
    void SetGpsType(int t);
    VGVehicleMission *GetVehicleMission()const;
    void reqMissionFromNet(int idx);
    void SetContinueCoor(const QGeoCoordinate &c, bool clear = false);
    QString GetQXAcc()const;
    QString GetQXPswd()const;
signals:
    void flightModeChanged();
    void planeIdChanged(const QString &id);
    void productTypeChanged(const QString &id);
    void organizationChanged(const QString &id);
    void managerChanged(const QString &id);
    void contactChanged(const QString &id);
    void medVolChanged(double f);
    void precisionChanged(double f);
    void posTipChanged();
    void accTipChanged();
    void gpsTipChanged();
    void satlateNumbChanged(int n);

    void bindedChanged(bool bBind);
    void statusChanged();
    void masterChanged(const QString &master);
    void userIdChanged(const QString &id);
    void directLinkChanged(bool isDirectLink);
    void landedChanged();
    void powPercentChanged(double per);
    void medSpeedChanged(double per);
    void compassAngleChanged(double f);
    void rollAngleChanged(double f);
    void pitchAngleChanged(double f);
    void coordinateChanged(const QGeoCoordinate &c);
    void heigthChanged(double f);
    void horSpeedChanged(double f);
    void verSpeedChanged(double f);
    void monitorChanged(bool b);
    void armChanged();
    void wlcLoseChanged();
    void missionValibleChanged(bool b);
    void missionChanged(VGVehicleMission *vm);
    void missionRun();
    void voltageChanged();
    void qxaccChanged();
    void qxpswdChanged();
protected slots:
    void onSysFinish(const QString &id);
    void onContentDestroied(QObject *o);
private:
    void onMissionRun();
    void readParameter(const QString &str);
    void setParameter(const QString &str, const QVariant &var);
    void saveMission();
    void loadMission();
    void sendCommand(int cmd, float f1 = 0, float f2 = 0, float f3 = 0, float f4 = 0, float f5 = 0, float f6 = 0, float f7 = 0);
    void sendSetHome(bool uav, const QGeoCoordinate &c=QGeoCoordinate());
    void prcsSupportAndHome();

    void _prcsArmOrDisarm(bool);
    void _prcsFligtMode(bool);
    void _prcsSetAssist(int, bool);
    void _prcsGetAssist(int);
    void _prcsSetHome(bool);
private:
    bool            m_bDirectLink;  //是否直连
    bool            m_bLanded;      //是否降落
    PlantStatus     m_status;
    bool            m_bBind;        //0 未绑定 1 绑定
    bool            m_bSychUav;
    bool            m_bSysBoudary;
    bool            m_bSys;

    QGeoCoordinate  m_lastPosition; //最后一次通信的坐标
    double          m_relativeAltitude;
    double          m_gndHeight;
    int             m_satNum;
    double          m_horSpeed;
    double          m_verSpeed;
    double          m_distance;
    double          m_compassAngle;
    double          m_rollAngle;
    double          m_pitchAngle;
    double          m_powerPercent;
	int				m_powerGrade;
    double          m_medicineSpeed;
    double          m_medicineVol;
	int				m_medicineGrade;
    double          m_precision; //航向精度
    int             m_tpPos;
    int             m_acc;
    int             m_gps;
    bool            m_bMonitor;
    bool            m_bMissionValible;
    bool            m_bArmOp;
    bool            m_bWlcLose;
    bool            m_bSetArm;
    float           m_voltage;
    int             m_missionCount;
    bool            m_bDownMs;

    qint64          m_bindTime;         //绑定时间
    qint64          m_unBindTime;       //解绑时间
    qint64          m_lastTime;         //最后在线时间
    qint64          m_lastLacalTime;    //无数据计时

    QString         m_planeId;          //飞机ID
    QString         m_productType;      //产品型号
    QString         m_organization;     //所属组织
    QString         m_manager;          //负责人
    QString         m_contact;          //联系方式
    QString         m_master;           //绑定者
    QString         m_userId;           //添加者的用户名
    QString         m_flightMode;       //飞行模式
    QString         m_flightModeSet;    //飞行模式设置
    QString         m_strQxAcc;         //千寻账号
    QString         m_strQxPswd;        //千寻密码
    VGVehicleMission        *m_uploadVm;  //上传mission
    VGCoordinate            *m_homeCoor;  //home
    VGCoordinate            *m_cntnCoor;  //中断点
    VGOutline               *m_planMissionLine;
    unsigned                m_baseMode;
    QMap<QString, QVariant> m_params;
    QMap<QString, QVariant> m_strLsReqParams;
    VGSupportPolyline       *m_rtl;
    uint32_t                m_statSupport;
};

#endif // VGMYPLANEINFORMATION_H
