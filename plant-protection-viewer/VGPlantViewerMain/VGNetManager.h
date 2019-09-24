#ifndef VGMAPCONTROL_H
#define VGMAPCONTROL_H

#include <QtPlugin>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "VGTcpClient.h"
#include "VGMacro.h"
#include "VGPlantInformation.h"
#include "mavlink_types.h"

class VGLandInformation;
class VGLandBoundary;
class VGFlyRoute;
class VGLandPolygon;
class VGVehicleMission;
class QThread;
class MissionItem;

namespace google {
    namespace protobuf {
        class MessageLite;
    }
}

namespace das {
    namespace proto {
        class OperationStatus;
        class ParcelSurveyInformation;
        class Outline;
        class GpsInformation;
        class UavProductInfo;
        class ParcelDescription;
        class OperationDescription;
        class OperationPlan;
        class OperationRoute;
        class UavAttitude;
        class OperationParams;
    }
}

class VGNetManager:public QObject
{
    Q_OBJECT
    //tcp连接是否正常
    //0 正在连接， 1 连接成功，2 未连接，-1 连接失败；
    Q_PROPERTY(int connectState READ GetConnectState NOTIFY connectStateChanged)
    Q_PROPERTY(QString host READ GetHost WRITE SetHost NOTIFY connectChanged)
    Q_PROPERTY(quint16 port READ GetHostPort WRITE SetHostPort NOTIFY connectChanged)
    Q_PROPERTY(QString userId READ GetUserId WRITE SetUserId NOTIFY sigUserIdChanged NOTIFY connectChanged)
    Q_PROPERTY(QString password READ GetPassword WRITE SetPassword NOTIFY connectChanged)
    Q_PROPERTY(QString check READ GetCheck  NOTIFY checkChanged)
    Q_PROPERTY(int regTip READ GetRegTip  NOTIFY regTipChanged)
    Q_PROPERTY(bool autoLoggin READ IsAutoLoggin WRITE SetAutoLoggin NOTIFY connectChanged)
public:
    VGNetManager();
    ~VGNetManager();

    //登陆服务器
	Q_INVOKABLE void loggin();
    Q_INVOKABLE void disConnect();          //客户端主动断开
public:
    int GetConnectState()const;
    QString GetUserId()const;

    bool    UploadItem(const MapAbstractItem *item);
    //无人机; type 0: 解除, 1: 绑定
    void    requestBindUav(const QString &uavId, int type);
    void    setDirectLinkUav(const QString &uavId);
    void    requestUavInfo(const QStringList &lstPlaneId);
    void    requestUavInfo(const QGeoCoordinate &coor, double dis);
    void    requestUavStatus(const QString &lstPlaneId);

    void    requestLandinfo(const QString &userId, const DescribeMap &bdid = DescribeMap()); //查询地块描述信息（包含地块信息）
    void    requestPlanning(const QString &pdid = QString(), const QString &odid = QString());
    void    requestOperationPlan(const QString &userId, const DescribeMap &bdid = DescribeMap()); //查询作业
    void    DeleteItemSafe(const MapAbstractItem *item);    //服务器删除
    //向无人机同步指定航线
    void    syncPlanning(VGVehicleMission *ms, const QString &uavId);
    void    SendControlUav(const QString &uav, const mavlink_message_t &mav);
    void    SendControlUav(const QString &uav, const QList<MissionItem*> &items);
    void    SendControlUav(const QString &uav, const QMap<QString, QVariant> &params);
protected:
    Q_INVOKABLE void registerUser(const QString &user, const QString &pswd = "", const QString &check = "");
    Q_INVOKABLE QString regTipStr(int tip);
    void    timerEvent(QTimerEvent *event);
    //登录
    QString GetHost()const;
    void    SetHost(const QString &host);
    quint16 GetHostPort()const;
    void    SetHostPort(quint16 port);
    void    SetUserId(const QString &user);
    QString GetPassword()const;
    void    SetPassword(const QString &pswd);
    bool    IsAutoLoggin()const;
    void    SetAutoLoggin(bool bAuto);
    //登陆状态（用户名和密码验证是否通过）
    void    setConnectState(int state);

    QString GetCheck()const;
    int GetRegTip()const;
    void logginServer();
private:
    //服务器数据
    void _processLoggin(const QByteArray &byte);
    void _processDelPDAck(const QByteArray &byte);
    void _processGps(const QByteArray &byte);
    void _processSurveyLands(const QByteArray &byte);
    void _processOparetionPlan(const QByteArray &byte);
    void _processUploadOperationPlan(const QByteArray &byte);
    void _processUploadSurveyResult(const QByteArray &byte);
    void _processUploadMissionResult(const QByteArray &byte);
    void _processPlantState(const QByteArray &byte);
    void _processPlantBind(const QByteArray &byte);
    void _processPlantInfo(const QByteArray &byte);
    void _processSysAck(const QByteArray &byte);
    void _processAckPostControl2Uav(const QByteArray &byte);
    void _processUav2Gc(const QByteArray &byte);
    void _processAckControl2Uav(const QByteArray &byte);
    void _processAckNewGS(const QByteArray &byte);
signals:
    void sigConnectServer(const QString &, int port, int = 3000, const QString & = "");
    void sigSendRequestInfo(const QString &, const QByteArray &arr);
    void connectStateChanged(int state);
    void autoReloadChanged(bool reload);
    void sigGpsInformation(const QString &id, const das::proto::GpsInformation &gps);
    void sigUavAttitude(const QString &id, const das::proto::UavAttitude &);
    void sigVGOperationStatus(const QString &id, const das::proto::OperationStatus &st);
    void sigVGOperationParams(const QString &id, const das::proto::OperationParams &p);

    void sigQueryItem(MapAbstractItem *item, const DescribeMap &parentDsc = DescribeMap());
    //无人机状态查询结果
    void sigQueryUavStatusSuccess(int result, const VGPlantInformation &pUavInfo);
    void sigUserIdChanged(const QString &usr);
    void sigSaveResult(const DescribeMap &dsc);
    void connectChanged();
    void sendUavErro(const QString &uav, int res);
    void sendFromUav(const QString &uav, const mavlink_message_t msg, bool finish);
    void sycRouteProcess(const QString &uav, int cout, int index);
    void checkChanged();
    void regTipChanged();
    void registed();
    void missionUploaded(const QString &uav);
private slots:
    void    sltTcpResult(bool connected);
    void    sltAckInfo(const QString &name, const QByteArray &byte);
    void    sltHeartTimeout();
private:
    void    readConfig();
    void    writeConfig(const QString &server, quint16 port, const QString &userId, const QString &password);
    bool    connectServer();//tcp连接
    //自动登陆
    void    _autoLoggin(bool bAuto);
    void    _checSeq();
    void    _sendNetWork(const google::protobuf::MessageLite &msg);

    ::das::proto::ParcelSurveyInformation *_boundaryToParcelSurvey(const VGLandBoundary *bdr);
    ::das::proto::Outline *_polygonToOutline(das::proto::Outline *outlin, const VGLandPolygon &plg);
    void _parseGPS(VGPlantInformation &pi, const das::proto::GpsInformation &gpsInfo);
    void _parsePlantInfo(VGPlantInformation &pi, const das::proto::UavProductInfo &plantInfo);
    VGLandInformation *_parseLand(const das::proto::ParcelDescription &pDes);
    VGLandBoundary *_parseBoundary(const das::proto::ParcelSurveyInformation &psi);
    VGFlyRoute *_parseFlyRoute(const das::proto::OperationDescription &od, DescribeMap &dsc);
    void _parseOperationRoute(const das::proto::OperationRoute &route);
    void _safeDeleteBoundary(const VGLandBoundary &bdr);
    void _safeDeleteFlyRoute(const VGFlyRoute &fr);

    int _getSendNo(const QString &name)const;
    void _setSendNo(const QString &name, int seq);
    das::proto::OperationPlan *_transOperationPlan(const VGFlyRoute &fr);
    void registerSuc();
    //上传测绘地块
    void uploadSurveyInfo(const VGLandBoundary &info);
    //上传规划
    void uploadPlanningRoute(const VGFlyRoute &fr);
private:
    VGTcpClient*        m_tcpClient;
    quint16             m_port;
    int                 m_nRegTip;
    int                 m_connectState;      //连接状态, 0 正在登陆， 1，成功， -1 登陆失败， 2 断开状态
    bool                m_autoLoggin;        //是否自动重连
    int                 m_tmAutoLoggin;     //自动登录定时器
    int                 m_nHeartTimes;      //心跳超时次数
    unsigned int        m_seqno;
    bool                m_isGetPlaneInfo;//是否获取无人机信息
    bool                m_bInitial;      //是否已获取作业
	QString             m_strServer;
	QString             m_strUserId;
    QString             m_strPassword;
    QString             m_strCheck;
    QStringList         m_strPdids;

	QTime               m_lastRevTime;      //最近接受到服务器数据时间
	QTimer              m_heartTimer;

	QThread             *m_thread;
	QMutex              m_mutex;

	QString             m_curUploadPlantID;     //当前上传的地块ID（本地ID）

    DescribeMap         m_mapDesUpload;

    QMap<QString, int> m_mapSendSeqs;
};

#endif // VGMAPCONTROL_H
