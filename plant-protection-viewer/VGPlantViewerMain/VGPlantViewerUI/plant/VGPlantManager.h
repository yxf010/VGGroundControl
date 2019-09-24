#ifndef __VG_PLANTMANAGER_H__
#define __VG_PLANTMANAGER_H__

#include <QObject>
#include "VGPlantInformation.h"
#include "mavlink_types.h"

//QML飞机管理
class VGNetManager;
class QGeoCoordinate;
class VGVehicleMission;

namespace das {
    namespace proto {
        class OperationStatus;
        class GpsInformation;
        class UavAttitude;
    }
}
class VGPlantManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(VGPlantInformation*  boundPlant READ GetBoundPlant CONSTANT) //绑定飞机
    Q_PROPERTY(VGPlantInformation* currentPlant READ GetCurrentPlant WRITE SetCurrentPlant NOTIFY currentPlantChanged)//当前选中的飞机
    Q_PROPERTY(bool oneKeyArm READ IsOneKeyArm WRITE SetOneKeyArm NOTIFY oneKeyArmChanged)
    Q_PROPERTY(bool voiceHeight READ IsVoiceHeight WRITE SetVoiceHeight NOTIFY voiceHeightChanged)
    Q_PROPERTY(bool voiceSpeed READ IsVoiceSpeed WRITE SetVoiceSpeed NOTIFY voiceSpeedChanged)
    Q_PROPERTY(bool voiceVS READ IsVoiceVS WRITE SetVoiceVS NOTIFY voiceVSChanged)
    Q_PROPERTY(bool voiceSat READ IsVoiceSat WRITE SetVoiceSat NOTIFY voiceSatChanged)
    Q_PROPERTY(bool connected READ IsConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool imGnd READ IsImitateGround NOTIFY imGndChanged)
public:
    explicit VGPlantManager(QObject *parent = 0);
    ~VGPlantManager();

    Q_INVOKABLE void addNewPlant(const QString &planeId, bool bMnt = false);
    Q_INVOKABLE void clearTmpPlant();
    Q_INVOKABLE void searchPlant(QGeoCoordinate &coor, double dis = 200);
    Q_INVOKABLE void remove(VGPlantInformation *pl);
    Q_INVOKABLE void testPump();
    Q_INVOKABLE void testMotor(int idx, int msTm = 1000, int speed = 1150);
    Q_INVOKABLE void shutdown();
    Q_INVOKABLE void disarm();
	Q_INVOKABLE void magnetic();
    Q_INVOKABLE bool setParameter(const QString &id, const QString &val);
    Q_INVOKABLE void setParameterIndex(const QString &id, int idx);
    Q_INVOKABLE QString getParamValue(const QString &key) const;
    Q_INVOKABLE int getParamIndex(const QString &key) const;

    void    startQueryMyPlaneStatus();
    void    stopQueryMyPlaneStatus();
    VGPlantInformation *GetPlantViaDrect()const;
    void    ViaDrectPlantCreate(const QString &mode);
    void    RemoveViaCloudPlant(const QString &planeId);
    VGPlantInformation *GetPlantById(const QString &id);
    void  Insert(VGPlantInformation *info, int i=-1);
    void  InitialNetWork(VGNetManager *nm);
    VGPlantInformation *GetCurrentPlant()const;
    void  SetCurPlant(const QString &id);
    VGPlantInformation *getExistPlaneBound(const QString &userId);
    bool   IsConnected()const;
    void   syncPlanFlight(VGVehicleMission *vm);
    bool   IsVoiceHeight()const;
    bool   IsVoiceSpeed()const;
    bool   IsVoiceVS()const;
    bool   IsVoiceSat()const;
    bool   IsOneKeyArm()const;
    void   DealParam(VGPlantInformation *info, const QString &id, const QVariant &var);
    bool   IsImitateGround()const;
public slots:
    void sltDeleteMyPlaneResult(int result, const QString &planeId, const QString &userId);
    void sltPlaneStsInformation(int st, const VGPlantInformation &splaneInfo);
    void sltVGOperationStatus(const QString &id, const das::proto::OperationStatus &st);
    void sltVGGpsInformation(const QString &id, const das::proto::GpsInformation &gpsInfo);
    void sltVGUavAttitude(const QString &id, const das::proto::UavAttitude &attInfo);
    void onNetStatchanged(int stat);
protected:
    void timerEvent(QTimerEvent *event);
    void SetCurrentPlant(VGPlantInformation *pl);
    VGPlantInformation *GetBoundPlant()const;

    VGPlantInformation *_getPlantByIdAndUser(const QString &id, const QString &user);

    void   SetOneKeyArm(bool b);
    void   SetVoiceHeight(bool b);
    void   SetVoiceSpeed(bool b);
    void   SetVoiceVS(bool b);
    void   SetVoiceSat(bool b);
    Q_INVOKABLE void writeConfig();
    Q_INVOKABLE void setQxAccount(const QString &acc, const QString &pswd);
    Q_INVOKABLE void getQxAccount();
    Q_INVOKABLE void clearQxAccount();
signals:
    void currentPlantChanged(VGPlantInformation *plant);
    void sigSavePlantInfo(VGPlantInformation *plant);
    void sigQueryPlaneInfo(const QStringList &plantIds);
    void sigPlaneStsInit(VGPlantInformation *plant);
    void sigQueryUavStatus(const QStringList &plants, const QStringList &uavIds);

    void oneKeyArmChanged(bool b);
    void voiceHeightChanged(bool b);
    void voiceSpeedChanged(bool b);
    void voiceVSChanged(bool b);
    void voiceSatChanged(bool b);
    void connectedChanged(bool b);
    void imGndChanged();
    void paramChanged(const QString &idStr);
protected slots:
    void onChildDestroyed(QObject *obj);
    void onPlantConnectChanged();
    void onSendUavErro(const QString &id, int res);
    void onSendFromUav(const QString &id, const mavlink_message_t &msg, bool finish);
    void onSycRouteProcess(const QString &uav, int count, int index);
protected:
    void _readConfig();
    void _writeConfig();
    void _addMonitorPant(const QString &id);

    void _prcsCmdAck(VGPlantInformation &p, const mavlink_message_t &m);
    void _prcsMissionCur(VGPlantInformation &p, const mavlink_message_t &m);
    void _prcsParamVal(VGPlantInformation &p, const mavlink_message_t &m);
    void _prcsMissionCount(VGPlantInformation &p, const mavlink_message_t &m, bool b);
    void _prcsMissionItem(VGPlantInformation &p, const mavlink_message_t &m, bool b);
    void _prcsHome(VGPlantInformation &p, const mavlink_message_t &m);
    void _prcsAssist(VGPlantInformation &p, const mavlink_message_t &m);
    void _prcsUavEvent(const QString &id, const mavlink_message_t &m);
    void _prcsQXAccount(VGPlantInformation &p, const mavlink_message_t &m);
    void _prcsQXCmd(VGPlantInformation &p, const mavlink_message_t &m);
private:
    QList <VGPlantInformation *>    m_palnts;
    VGPlantInformation              *m_plantBound;                 //当前绑定的飞机
    VGPlantInformation              *m_plantCur;                   //当前飞机
    bool                            m_bExistBound;
    bool                            m_bInitNet;
    QStringList                     m_mntPants;
    QString                         m_strCur;
    int                             m_idTimerQureryPlant;
    bool                            m_bOneKeyArm;
    bool                            m_bMission;
    unsigned                        m_voice;
};

#endif // __VG_PLANTMANAGER_H__
