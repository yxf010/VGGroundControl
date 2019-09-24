#ifndef VGVEHICLEMANAGER_H
#define VGVEHICLEMANAGER_H

#include <QObject>
#include <QTimer>
#include "LinkInterface.h"
#include "VGVehicle.h"
#include <QList>

class VGVehicleManager : public QObject
{
    Q_OBJECT
public:
    explicit VGVehicleManager(QObject *parent = 0);

public:
    VGVehicle* getVehicleById(int vehicleId);
    VGVehicle* activeVehicle()const;
    VGVehicle* surveyVehicle()const;
private:
    void setActiveVehicle(VGVehicle* vehicle);
signals:
    void    sigMavlinkMessageReceived(LinkInterface* link, const mavlink_message_t &message);
    void    sigActiveVehicleAvailableChanged(bool active);      //当前飞机可用或者不可用
    void    sigActiveVehicleChanged(VGVehicle* activeVehicle);    //当前飞机发生变化

    void    sigUpdateLogList(uint32_t time_utc, uint32_t size, uint16_t id, uint16_t num_logs, bool downloaded);
    void    sigReceiveLogData(uint32_t ofs, uint16_t id, uint8_t count, const uint8_t* data);
public slots:
    void    onHeartbeat(LinkInterface* link, int vehicleId, int vehicleMavlinkVersion, int vehicleFirmwareType, int vehicleType);
    //查询日志命令返回
    void    sltUpdateLogList(VGVehicle* vehicle, uint32_t time_utc, uint32_t size, uint16_t id, uint16_t num_logs);
    //请求下载日志
    void    sltRequestLogData(int vehicleId, int logId, ulong offset, ulong count);
    //收到日志数据
    void    sltReceiveLogData(VGVehicle* vehicle, uint32_t ofs, uint16_t id, uint8_t count, const uint8_t* data);
private slots:
    void    sltVehicleAttitudeChanged(VGVehicle* vehicle, double roll, double pitch, double yaw);
    void    sltVehicelSpeedChanged(VGVehicle* vehicle, double speedX, double speedY);
    void    sltVehicelGpsChanged(double lat, double lon, double alt, int nSatNum);
    void    onPosTypeChanged(VGVehicle *vehicle, int fix);
    void    sltVehicelAltitudeChanged(VGVehicle* vehicle, double altitude);
    void    sltGndHeightChanged(VGVehicle* vehicle, double height);
    void    sltVehicelBatteryStatus(VGVehicle* vehicle, int batterry, uint16_t mv, uint8_t warn);
    void    onPrecisionChanged(VGVehicle* vehicle, double pre);
    void    sltSetActiveVehiclePhase2(void);        //延迟设定当前生效飞机
    void    sltSendVGHeartbeat();                   //定时向飞控发送心跳包
    void    sltConnectionLostChanged(bool b);
    void    onNewMissionItemsAvailable(bool b, MAV_MISSION_TYPE tp);
    void    onSysAvalibleChange(bool b);
    void    onModeChanged(const QString &mode);
    void    onMavCommandResult(MAV_CMD cmd, bool res);
    void    onMissionPrcs(double par, bool b);
    void    onPrcCurrentIndex(int curIndex);
    void    onPrcLastIndex(int curIndex);
    void    onVehicleUavIdChanged(VGVehicle *v, const QString &uavid);
    void    onVehicelDistanceChanged(VGVehicle* vehicle, double distance);
    void    onHomeChanged(const QGeoCoordinate &coor);
    void    onSyschFinish(bool error);
    void    onParamResult(const QString &id, const QVariant &var);
    void    onSprayGot(VGVehicle *v, double speed, double vol, uint8_t stat, uint8_t mode);
	void	onVehicleCog(int cog);
    void    onSysStatus(uint8_t st);
    void    onAssistChanged(const QGeoCoordinate &, const QGeoCoordinate &, const QGeoCoordinate &, int);
    void    onEventChanged(const QString &id, int evt);
private:
    QList<VGVehicle*>   _vehicles;
    VGVehicle*          _activeVehicle;                     ///< Currently active vehicle
    bool                _activeVehicleAvailable;            ///< true: An active vehicle is available
    VGVehicle*          _vehicleBeingSetActive;         ///< Vehicle being set active in queued phases

    QTimer              _vgsHeartbeatTimer;             ///< Timer to emit heartbeats
    bool                _vgsHeartbeatEnabled;           ///< Enabled/disable heartbeat emission
    int                 m_nBatteryAlarm;
    int                 m_bAlarmFirst;
};

#endif // VGVEHICLEMANAGER_H
