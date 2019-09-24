#ifndef VGVEHICLE_H
#define VGVEHICLE_H

#include <QObject>
#include <QGeoCoordinate>
#include "LinkInterface.h"
#include <QPair>
#include <QTimer>
#include <QPointF>
#include <mavlink.h>

#define MAVTYPESURVEY   MAV_TYPE_VTOL_RESERVED2

class MissionManager;
class VGVehicleMission;
class VGOutline;
class VGCoordinate;
class MissionItem;

class VGVehicle : public QObject
{
    Q_OBJECT
public:
    typedef struct {
        int     component;
        MAV_CMD command;
        float   rgParam[7];
        bool    bInt;
    } MavCommandQueueEntry_t;
    typedef QPair<VGVehicleMission *, bool> PairMission;
public:
    explicit VGVehicle(QObject *parent = 0);
    VGVehicle(LinkInterface*    link,
			  int               vehicleId,
			  MAV_AUTOPILOT     firmwareType,
              MAV_TYPE          vehicleType);
    ~VGVehicle();
public:
    int id()const;
    int firewareType()const;
    MAV_TYPE vehicleType()const;

    /// Sends a message to all links accociated with this vehicle
    void sendMessage(const mavlink_message_t &message);
    void sendRTCM(const QList<uint16_t> idxs, const QList<QByteArray> &rtcms);
    /// Sends a message to the specified link
    /// @return true: message sent, false: Link no longer connected
    bool sendMessageOnLink(LinkInterface* link, mavlink_message_t message);

    /// Returns the highest quality link available to the Vehicle
    LinkInterface* priorityLink(void);

    //设置是否检测链接超时
    void setConnectionLostEnabled(bool connectionLostEnabled);
    void setAutoDisconnect(bool autoDisconnect);
    void disconnectInactiveVehicle(void);       //断开链接

    QString uavid()const;
	void setHomePosition(QGeoCoordinate& homeCoord);
	void SetFlightMode(const QString &fm);
	bool supportsMissionItemInt()const;
    bool syncPlanFlight(VGVehicleMission *mission, bool bBdr=false);
    void setQxAccount(const QString &acc, const QString &pswd);
    bool isMissionIdle()const;
    void loadFromVehicle(MAV_MISSION_TYPE tp=MAV_MISSION_TYPE_MISSION);
    const QList<MissionItem*> &missionItems()const;
    bool hasValidMissionItems()const;
    MissionManager *missionManager()const;
    void setArmed(bool armed);
    void sendMavCommand(MAV_CMD command, float param1 = 0.0f, float param2 = 0.0f,float param3 = 0.0f
        , float param4 = 0.0f, float param5 = 0.0f, float param6 = 0.0f, float param7 = 0.0f);
    void sendMavCommandInt(MAV_CMD command, float param1 = 0.0f, float param2 = 0.0f, float param3 = 0.0f
        , float param4 = 0.0f, int param5 = 0.0f, int param6 = 0.0f, float param7 = 0.0f);
    void setVehicleParameter(const QString &strParam, const QVariant &param);
    void getVehicleParameter(const QString &strParam);
	QString flightMode()const;
public:
    static QStringList	GetFlightModes();
protected:
    double calculateHoverRadius()const;//悬停半径
    double calculateHoverTime(double radius)const;//悬停时间
    void   timerEvent(QTimerEvent *event);
protected slots:
    void    _mavlinkMessageReceived(LinkInterface* link, const mavlink_message_t &message);
    void    _onMissionSych(bool bError);
    void    _sendMavCommandAgain();
    void    onMissionError(int errorCode, const QString& errorMsg);
    void    onMissionDestroed(QObject *obj);

    void    sltRequestLogList(int start, int end);
    //请求下载日志数据
    void    sltRequestLogData(int logId, long offset, long count);
    void    _linkInactiveOrDeleted(LinkInterface* link);
    void    _sendMessage(const mavlink_message_t &message);
    void    _sendMessageOnLink(LinkInterface* link, const mavlink_message_t &message);
    void    _connectionLostTimeout(void);          //连接超时定时器
private:
    bool _containsLink(LinkInterface* link);
    void _addLink(LinkInterface* link);
    bool _contains(VGVehicleMission *ms, bool bBdr=false);
    QGeoCoordinate _vehiclePostion()const;

    quint64 getUnixReferenceTime(quint64 time);    

    void _handleHeartbeat(const mavlink_message_t& message);
    void _handleFlyAttitude(const mavlink_message_t& message);
    void _handleAttitudeQuaternion(const mavlink_message_t& message);
    void _handleAutopilotVersion(LinkInterface* link, const mavlink_message_t& message);
    void _handleHomePosition(const mavlink_message_t& message);
    void _handleGlobalPositon(const mavlink_message_t& message);
    void _handleSysStatus(const mavlink_message_t& message);
    void _handleGpsSatellite(const mavlink_message_t& message);
    void _handleGpsStatus(const mavlink_message_t& message);
    void _handleAltitude(const mavlink_message_t& message);
    void _handleVfrHud(const mavlink_message_t& message);
    void _handleBattery(const mavlink_message_t& message);
    void _handleLogEntry(const mavlink_message_t& message);
    void _handleLogData(const mavlink_message_t& message);
    void _handleParameter(const mavlink_message_t& message);
    void _handleCommandAck(const mavlink_message_t& message);
    void _handleSpray(const mavlink_message_t& message);
    void _handleSupport(const mavlink_message_t& message);
    void _handleEvent(const mavlink_message_t& message);
signals:
    /// Used internally to move sendMessage call to main thread
    void    _sendMessageOnThread(const mavlink_message_t &message);
    void    _sendMessageOnLinkOnThread(LinkInterface* link, const mavlink_message_t &message);
    //姿态发生变化
    void    attitudeChanged(VGVehicle* vehicle, double roll, double pitch, double yaw);
    void    speedChanged(VGVehicle* vehicle, double speedH, double speedV);
    void    postionChanged(double lat, double lon, double alti, int nSatNum);
    void    altitudeChanged(VGVehicle* vehicle, double altitude);
    void    gndHeightChanged(VGVehicle* vehicle, double height);
    void    batterryStatus(VGVehicle* vehicle, int battery, uint16_t v, uint8_t btrWarn);
    void    distanceChanged(VGVehicle* vehicle, double distance);
    void    precisionChanged(VGVehicle* vehicle, double pre);
    void    mavCommandResult(MAV_CMD command, bool bSuc);
    //断开链接
    void    connectionLostChanged(bool connectionLost);
    //飞控返回的日志查询信号；
    void    sigUpdateLogList(VGVehicle* vehicle, uint32_t time_utc, uint32_t size, uint16_t id, uint16_t num_logs);
    void    sigReceiveLogData(VGVehicle* vehicle, uint32_t ofs, uint16_t id, uint8_t count, const uint8_t* data);

    void    vehicleUavIdChanged(VGVehicle *v, const QString &uavid);
	void	mavlinkMessageReceived(const mavlink_message_t& message);
	void	flightModeChanged(const QString &);
    void    sysAvalibleChanged(bool b);
    void    paramResult(const QString &id, const QVariant &var);
    void    homePositionChanged(const QGeoCoordinate &c);
    void    posTypeChanged(VGVehicle *v, int fix);
    void    sychMissionFinish(bool suc);
    void    sprayGot(VGVehicle *v, double speed, double vol, uint8_t stat, uint8_t mode);
	void	vehicleCog(int);
    void	sysStatus(uint8_t st);
    void	assistChanged(const QGeoCoordinate &, const QGeoCoordinate &, const QGeoCoordinate &cCtn, int);
    void	vehicleEvent(const QString &id, int evt);
private:
    friend class VGVehicleManager;

    int                     m_vehicleID;		//mavlink system ID, vehicle id
    int                     m_timerId;
    unsigned                m_countTimer;
    MAV_TYPE		        m_vehicleType;		//设备类型
    MAV_AUTOPILOT           m_firewareType;		//固件类型
    QList<LinkInterface*>   m_links;

    bool m_allLinksInactiveSent;         //< true: allLinkInactive signal already sent one time

    quint64 m_onboardTimeOffset;
    qint64 m_tmLastRtcm;
    bool m_attitudeKnown;                 ///< True if attitude was received, false else

    QGeoCoordinate  m_homePosition;      //home坐标
    int     m_gpsCount;                  //GPS可用星数
    double  m_distance;                  //离home点的距离

    double m_roll;
    double m_pitch;
    double m_yaw;
	double m_acceleration;      //最大加速度
	double m_speedMax;			//最大速度
    double m_groundSpeed;          ///< Groundspeed 水平速度
    double m_verSpeed;            //垂直方向速度

    double m_latitude;            ///< Global latitude as estimated by position estimator
    double m_longitude;           ///< Global longitude as estimated by position estimator
    //相对高度
    double m_altitudeRelative;    ///< Altitude above home as estimated by position estimator
    //绝对高度（海平面高度）
    double m_altitude;

    uint8_t				m_btrWarn;
    uint8_t				m_baseMode;
    uint16_t		    m_voltage;
	uint32_t			m_customMode;
    // Lost connection handling
    bool                m_connectionLost;            //连接是否丢失
    bool                m_connectionLostEnabled;     //是否启用断线检测
    bool                m_globalPositionIntMessageAvailable;
    QTimer              m_connectionLostTimer;
    bool                m_autoDisconnect;            //是否自动清除link
    QString             m_uavID;
    QString				m_modeSet;
	bool			    m_supportsMissionItemInt;
    bool                m_bArm;
	MissionManager		*m_mgrMission;				//mavlink任务管理，如上传下载航迹
    QTimer                          m_mavCommandAckTimer;
    QList<MavCommandQueueEntry_t>   m_mavCommandQueue;
    int                             m_mavCommandRetryCount;
    QList<PairMission>              m_pfInfos;               //同步的航迹
};

#endif // VGVEHICLE_H
