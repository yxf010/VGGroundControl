#include "VGNetManager.h"
#include <QDebug>
#include <QThread>
#include "VGGlobalFun.h"
#include "VGMacro.h"
#include "VGLandInformation.h"
#include "VGCoordinate.h"
#include "VGLandPolygon.h"
#include "VGOutline.h"
#include "VGFlyRoute.h"
#include "VGLandBoundary.h"
#include "VGApplication.h"
#include "VGGlobalFun.h"
#include "VGMavLinkCode.h"
#include "MissionItem.h"
#include "VGPlantManager.h"
#include "VGPlantInformation.h"
#include "VGVehicleMission.h"
#include "das.pb.h"

#define d_p_ClassName(_cls) QString(_cls::descriptor()->full_name().c_str())
#define INTCOORLATORLONPOW   1e7

using namespace das::proto;

static void _toPolygon(const Outline &ol, VGLandPolygon &plg)
{
    int nCount = ol.coordinates_size();
    for (int i = 0; i < nCount; i++)
    {
        Coordinate coordinateOutline = ol.coordinates(i);
        QGeoCoordinate coordinate;
        coordinate.setLatitude(double(coordinateOutline.latitude()) / INTCOORLATORLONPOW);
        coordinate.setLongitude(double(coordinateOutline.longitude()) / INTCOORLATORLONPOW);
        coordinate.setAltitude(double(coordinateOutline.altitude()) / 1e3);

        int tp = plg.GetId() == VGLandPolygon::Boundary ? VGCoordinate::Boundary : VGCoordinate::BlockBoundary;   
        plg.AddCoordinate(coordinate, tp);
    }
}

static SurveryPrecision transform2Precision(MapAbstractItem::SurveyType tp)
{
    switch (tp)
    {
    case MapAbstractItem::Survey_GPS:
        return MEDIUM;
    case MapAbstractItem::Survey_Vehicle:
        return HIGH;
    default:
        break;
    }
    return LOW;
}

static MapAbstractItem::SurveyType transform2SurveyType(SurveryPrecision t)
{
    switch (t)
    {
    case HIGH:
        return MapAbstractItem::Survey_Vehicle;
    case MEDIUM:
        return MapAbstractItem::Survey_GPS;
    default:
        break;
    }
    return MapAbstractItem::Survey_DrawMap;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
//VGNetManager
///////////////////////////////////////////////////////////////////////////////////////////////////////
VGNetManager::VGNetManager() :QObject(), m_tcpClient(new VGTcpClient)
, m_seqno(0), m_port(0), m_tmAutoLoggin(-1), m_autoLoggin(true)
, m_nHeartTimes(0), m_connectState(2), m_nRegTip(2)
, m_thread(new QThread), m_bInitial(false)
{
    readConfig();

    m_heartTimer.setInterval(HEART_INTERVAL * 1000);
    connect(&m_heartTimer, &QTimer::timeout, this, &VGNetManager::sltHeartTimeout);

    if (m_thread && m_tcpClient)
    {
        m_tcpClient->moveToThread(m_thread);
        connect(m_tcpClient, &VGTcpClient::sigCommunicateResult, this, &VGNetManager::sltTcpResult);
        connect(m_tcpClient, &VGTcpClient::sigInfoAck, this, &VGNetManager::sltAckInfo);

        connect(this, &VGNetManager::sigSendRequestInfo, m_tcpClient, &VGTcpClient::sltSendRequestInfo);
        connect(this, &VGNetManager::sigConnectServer, m_tcpClient, &VGTcpClient::connetServer);
        m_thread->start();
        setConnectState(2);
    }
}

VGNetManager::~VGNetManager()
{
    m_thread->terminate();
    delete m_thread;
    delete m_tcpClient;
}

void VGNetManager::readConfig()
{
    if (QSettings *settings = qvgApp->GetSettings())
    {
        settings->beginGroup("config");
        m_autoLoggin = settings->value("autoLoggin", true).toBool();
        m_strServer = settings->value("serverIp", "101.200.55.175").toString();
        m_port = settings->value("port", "8198").toString().toInt();
        m_strUserId = settings->value("user", "").toString();
        m_strPassword = settings->value("password", "").toString();
        settings->endGroup();
        emit sigUserIdChanged(m_strUserId);
        _autoLoggin(m_autoLoggin);
    }
}

void VGNetManager::setConnectState(int state)
{
    m_connectState = state;
    emit connectStateChanged(state);
    if (m_connectState == 1 && !m_bInitial)
        requestLandinfo(m_strUserId);
}

QString VGNetManager::GetCheck() const
{
    return m_strCheck;
}

int VGNetManager::GetRegTip() const
{
    return m_nRegTip;
}

void VGNetManager::registerUser(const QString &user, const QString &pswd, const QString &check)
{
    if(connectServer())
    {
        m_strUserId = user;
        RequestNewGS gs;
        _checSeq();
        gs.set_seqno(m_seqno);
        gs.set_userid(user.toUtf8().data());
        if (!pswd.isEmpty() && !check.isEmpty())
        {
            m_strPassword = pswd;
            gs.set_password(pswd.toUtf8().data());
            gs.set_check(check.toUtf8().data());
        }

        _sendNetWork(gs);
    }
}

QString VGNetManager::regTipStr(int tip)
{
    switch (tip)
    {
    case -2:
        return QString::fromStdWString(L"用户名不合法");
    case -1:
        return QString::fromStdWString(L"服务器出错");
    case 0:
        return QString::fromStdWString(L"用户名已存在");
    case 1:
        return QString::fromStdWString(L"用户名可用");
    default:
        return QString::fromStdWString(L"字母,数字,_组合(3-24)");
    }
}

void VGNetManager::logginServer()
{
    RequestGSIdentityAuthentication cmdRgsia;
    _checSeq();

    cmdRgsia.set_seqno(m_seqno);
    cmdRgsia.set_userid(m_strUserId.toUtf8().data());
    cmdRgsia.set_password(m_strPassword.toUtf8().data());
    _sendNetWork(cmdRgsia);
}

void VGNetManager::SetHostPort(quint16 port)
{
    m_port = port;
}

void VGNetManager::SetUserId(const QString &user)
{
    if (m_strUserId == user)
        return;

    m_strUserId = user;
    emit sigUserIdChanged(user);
}

QString VGNetManager::GetPassword() const
{
    return m_strPassword;
}

void VGNetManager::SetPassword(const QString &pswd)
{
    m_strPassword = pswd;
}

bool VGNetManager::IsAutoLoggin()const
{
    return m_autoLoggin;
}

void VGNetManager::SetAutoLoggin(bool bAuto)
{
    m_autoLoggin = bAuto;
    _autoLoggin(bAuto);
}

void VGNetManager::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != m_tmAutoLoggin)
        return QObject::timerEvent(e);

    if ( m_connectState != 0
      && m_connectState != 1
      && !m_strUserId.isEmpty()
      && !m_strPassword.isEmpty() )
        loggin();

    e->accept();
}

QString VGNetManager::GetHost() const
{
    return m_strServer;
}

void VGNetManager::SetHost(const QString &host)
{
    m_strServer = host;
}

quint16 VGNetManager::GetHostPort() const
{
    return m_port;
}

void VGNetManager::writeConfig(const QString &server, quint16 port, const QString &userId, const QString &password)
{
    if (QSettings *settings = qvgApp->GetSettings())
    {
        settings->beginGroup("config");
        settings->setValue("serverIp", server);
        settings->setValue("port", port);
        settings->setValue("user", userId);
        settings->setValue("password", password);
        settings->setValue("autoLoggin", m_autoLoggin);
        settings->endGroup();
    }
}

bool VGNetManager::connectServer()
{
    if (m_strServer.isEmpty()
        || m_port <= 0
        || m_connectState == 1
        || !m_tcpClient)
        return false;

    m_tcpClient->InitSock();
    writeConfig(m_strServer, m_port, m_strUserId, m_strPassword);
    setConnectState(0);
    emit sigConnectServer(m_strServer, m_port);
    return true;
}

void VGNetManager::_processLoggin(const QByteArray &byte)
{
    AckGSIdentityAuthentication ack;
    ack.ParseFromArray(byte.data(), byte.size());
    int result = ack.result();
    if (result == 1 && m_seqno == ack.seqno())
    {
        setConnectState(1);
        m_heartTimer.start();
    }
    else if (result != 1)
    {
        m_tcpClient->DisconnectTcp();
        setConnectState(-1);
        _autoLoggin(false);
    }
}

void VGNetManager::_processDelPDAck(const QByteArray &)
{
}

void VGNetManager::_processGps(const QByteArray &byte)
{
    AckOperationInformation ackOi;
    bool isParsePassed = false;
    static PostOperationInformation cmdPoi;
    if (cmdPoi.ParseFromArray(byte.data(), byte.size()))
    {
        isParsePassed = true;
        ackOi.set_seqno(cmdPoi.seqno());
        ackOi.set_result(1);

        PostOperationInformation* message = &cmdPoi;
        int messageCount = message->oi_size();
        m_mutex.lock();

        for (int i = 0; i < messageCount; i++)
        {
            OperationInformation *oi = message->mutable_oi(i);
            QString uavid(oi->uavid().c_str());

            if (oi->has_gps())
                emit sigGpsInformation(uavid, oi->gps());

            if (oi->has_attitude())
                emit sigUavAttitude(uavid, *oi->mutable_attitude());

            if (oi->has_status())
                emit sigVGOperationStatus(uavid, *oi->mutable_status());

            if (oi->has_params())
                emit sigVGOperationParams(uavid, *oi->mutable_params());
        }
        m_mutex.unlock();
    }

    if (!isParsePassed)
    {
        ackOi.set_seqno(0x7FFFFFFF);
        ackOi.set_result(0);
    }
    _sendNetWork(ackOi);
}

void VGNetManager::_processUploadSurveyResult(const QByteArray &byte)
{
    static AckPostParcelDescription sAckPpdes;
    if (sAckPpdes.ParseFromArray(byte, byte.size()))
    {
        int result = sAckPpdes.result();
        if (sAckPpdes.seqno() == _getSendNo(d_p_ClassName(PostParcelDescription)) && result==1)
        {
            QString surveyId = QString(sAckPpdes.pdid().c_str());
            QString plantDesId = m_curUploadPlantID;
            QString boundaryId = QString(sAckPpdes.psiid().c_str());
            QString ownerId = QString(sAckPpdes.pcid().c_str());

            m_mapDesUpload[LandID] = surveyId;
            m_mapDesUpload[BoundaryId] = boundaryId;
            emit sigSaveResult(m_mapDesUpload);
        }
        m_mapDesUpload.clear();
    }
}

void VGNetManager::_processSurveyLands(const QByteArray &byte)
{
    AckRequestParcelDescriptions sAckPDess;
    if (sAckPDess.ParseFromArray(byte.data(), byte.size()))
    {
        if (sAckPDess.seqno() == _getSendNo(d_p_ClassName(RequestParcelDescriptions)))
        {
            if (sAckPDess.result() != 1)
                return;

            for (int i = 0; i < sAckPDess.pds_size(); i++)
            {
                DescribeMap dsc;
                if (VGLandInformation *land = _parseLand(sAckPDess.pds(i)))
                {
                    VGGlobalFunc::initialItemDescribe(dsc, *land);
                    emit sigQueryItem(land, dsc);
                }
                if (VGLandBoundary *bdr = _parseBoundary(sAckPDess.pds(i).psi()))
                {
                    VGGlobalFunc::initialItemDescribe(dsc, *bdr);
                    emit sigQueryItem(bdr, dsc);
                }
            }
        }
        if (!m_bInitial)
            requestPlanning();
        m_bInitial = true;
    }
}

void VGNetManager::_processOparetionPlan(const QByteArray &byte)
{
    AckRequestOperationDescriptions ackRODess;  //应答作业描述信息
    if (ackRODess.ParseFromArray(byte.data(), byte.size()))
    {
        if (ackRODess.seqno() != _getSendNo(d_p_ClassName(RequestOperationDescriptions)) || ackRODess.result() != 1)
            return;

        int odsCount = ackRODess.ods_size();
        for (int i = 0; i < odsCount; i++)
        {
            DescribeMap dsc;
            if (VGFlyRoute *fr = _parseFlyRoute(ackRODess.ods(i), dsc))
                emit sigQueryItem(fr, dsc);
        }
    }
}

void VGNetManager::_processUploadOperationPlan(const QByteArray &byte)
{
    AckPostOperationDescription ackPor;          //上传规划应答
    if (ackPor.ParseFromArray(byte.data(), byte.size()))
    {
        int result = ackPor.result();
        if (result == 1)
            m_mapDesUpload["routeId"] = ackPor.odid().c_str();

        emit sigSaveResult(m_mapDesUpload);
        m_mapDesUpload.clear();
    }
}

void VGNetManager::_processPlantState(const QByteArray &byte)
{
    static AckRequestUavStatus sAckRUS;                 //应答查询无人机状态
    if (sAckRUS.ParseFromArray(byte, byte.size()))
    {
        bool isFound = false;
        int uavSize = sAckRUS.status_size();
        for (int i = 0; i < uavSize; i++)
        {
            UavStatus uavStatus(sAckRUS.status(i));

            QString uavId = uavStatus.uavid().c_str();
            int result = uavStatus.result();
            if (!uavId.isEmpty())
            {
                VGPlantInformation planeInfo;
                planeInfo.SetStatus(VGPlantInformation::UnConnect);
                planeInfo.setPlaneId(uavId);
                planeInfo.setMaster(uavStatus.binder().c_str());
                planeInfo.setUserId(m_strUserId);
                planeInfo.SetBinded(uavStatus.binded());
                if (uavStatus.timestamp() > 0)
                    planeInfo.setLastTime(uavStatus.timestamp());
                else
                    planeInfo.setLastTime(0);

                if (uavStatus.bindtime() > 0)
                    planeInfo.setBindTime(uavStatus.bindtime());
                else
                    planeInfo.setBindTime(0);

                if (uavStatus.unbindtime())
                    planeInfo.setUnBindTime(uavStatus.unbindtime());
                else
                    planeInfo.setUnBindTime(0);

                emit sigQueryUavStatusSuccess(result, planeInfo);
            }
        }
    }
}

void VGNetManager::_processPlantBind(const QByteArray &byte)
{
    static AckRequestBindUav sAckRBS;                 //应答绑定无人机
    if (sAckRBS.ParseFromArray(byte.data(), byte.size()))
    {
        int result = sAckRBS.result();
        const UavStatus &uavStatus = sAckRBS.status();
        int type = sAckRBS.opid();
        QString uavId = uavStatus.uavid().c_str();
        VGPlantInformation planeInfo;
        bool bOnline = uavStatus.online();
        QString master = uavStatus.binder().c_str();

        _parseGPS(planeInfo, uavStatus.pos());
        planeInfo.SetBinded(type == 1);
        if (uavStatus.timestamp() > 0)
            planeInfo.setLastTime(uavStatus.timestamp());
        else
            planeInfo.setLastTime(0);

        if (uavStatus.bindtime() > 0)
            planeInfo.setBindTime(uavStatus.bindtime());
        else
            planeInfo.setBindTime(0);

        if (uavStatus.unbindtime())
            planeInfo.setUnBindTime(uavStatus.unbindtime());
        else
            planeInfo.setUnBindTime(0);

        planeInfo.SetStatus(bOnline ? VGPlantInformation::Connected : VGPlantInformation::UnConnect);
        planeInfo.setPlaneId(uavId);
        planeInfo.setMaster(master);

        emit sigQueryUavStatusSuccess(result, planeInfo);
    }
    else
    {
        qDebug() << "Parse protobuf is false";
    }
}

void VGNetManager::_processPlantInfo(const QByteArray &byte)
{
    AckRequestUavProductInfos aackRUPI;            //应答查询无人机信息
    if (aackRUPI.ParseFromArray(byte.data(), byte.size()))
    {
        if (!m_isGetPlaneInfo)
            m_isGetPlaneInfo = true;
        if (aackRUPI.seqno() == _getSendNo(d_p_ClassName(RequestUavProductInfos)))
        {
            int result = aackRUPI.result();
            VGPlantInformation planeInfo;
            if (result == 1)
            {
                int planeSize = aackRUPI.upi_size();
                if (planeSize > 0)
                {
                    for (int i = 0; i < planeSize; i++)
                    {
                        _parsePlantInfo(planeInfo, aackRUPI.upi(i));
                        emit sigQueryUavStatusSuccess(result, planeInfo);
                    }
                    return;
                }
            }
            emit sigQueryUavStatusSuccess(-2, planeInfo);
        }
    }
}

void VGNetManager::_processSysAck(const QByteArray &byte)
{
    SyscOperationRoutes sAckRUOR;       //向无人机同步指定航线应答
    if (sAckRUOR.ParseFromArray(byte.data(), byte.size()))
    {
        QString uav = sAckRUOR.has_uavid() ? QString::fromUtf8(sAckRUOR.uavid().c_str()):QString();
        int nCount = sAckRUOR.result() < 0 ? -1 : sAckRUOR.count();
        int index = nCount < 0 ? -1 : sAckRUOR.index();
        emit sycRouteProcess(uav, nCount, index);
    }
}

void VGNetManager::_processAckPostControl2Uav(const QByteArray &byte)
{
    AckPostControl2Uav sAck;
    if (sAck.ParseFromArray(byte.data(), byte.size()))
    {
        if (sAck.result() != 1)
        {
            QString strId = QString::fromUtf8(sAck.uavid().c_str());
            emit sendUavErro(strId, sAck.result());
        }
    }
}

void VGNetManager::_processUav2Gc(const QByteArray &byte)
{
    PostStatus2GroundStation uav2Gc;
    if (uav2Gc.ParseFromArray(byte.data(), byte.size()))
    {
        QString id = QString::fromUtf8(uav2Gc.uavid().c_str());
        for (int i = 0; i < uav2Gc.data_size(); ++i)
        {
            const ::std::string &dt = uav2Gc.data(i);
            if (dt.size() < 3)
                return;

            mavlink_message_t msg = {0};
            msg.msgid = *(uint16_t*)dt.c_str();
            msg.len = uint8_t(dt.size())-sizeof(uint16_t);
            memcpy(msg.payload64, dt.c_str() + sizeof(uint16_t), msg.len);
            emit sendFromUav(id, msg, i==uav2Gc.data_size()-1);
        }
    }
}

void VGNetManager::_processAckControl2Uav(const QByteArray &byte)
{
    AckPostControl2Uav ack;
    if (ack.ParseFromArray(byte.data(), byte.size()))
    {
    }
}

void VGNetManager::_processAckNewGS(const QByteArray &byte)
{
    AckNewGS ackGs;
    if (ackGs.ParseFromArray(byte.data(), byte.size()))
    {
        m_strCheck.clear();
        if (ackGs.has_check())
        {
            m_strCheck = ackGs.check().c_str();
            emit checkChanged();
        }
        if (m_nRegTip != ackGs.result())
        {
            m_nRegTip = ackGs.result();
            emit regTipChanged();
        }
        if (m_nRegTip == 3)
            qvgApp->SetQmlTip(QString::fromStdWString(L"验证码错误"));
        else if (m_strCheck.isEmpty() && 1 == m_nRegTip)
            registerSuc();
    }
}

void VGNetManager::_parseGPS(VGPlantInformation &pi, const GpsInformation &gpsInfo)
{
    double lon = (double)gpsInfo.longitude() /INTCOORLATORLONPOW;
    double lat = (double)gpsInfo.latitude() / INTCOORLATORLONPOW;
    double altitude = (double)gpsInfo.altitude() / (float)1e3;

    ShareFunction::gpsCorrect(lat, lon);
    QGeoCoordinate coordinate;
    coordinate.setLatitude(lat);
    coordinate.setLongitude(lon);
    coordinate.setAltitude(altitude);
    pi.setCoordinate(coordinate);
}

void VGNetManager::_parsePlantInfo(VGPlantInformation &pi, const UavProductInfo &plantInfo)
{
    pi.setPlaneId(plantInfo.uavid().c_str());
    pi.setProductType(plantInfo.producttype().c_str());
    pi.setManager(plantInfo.manager().c_str());
    pi.setContact(plantInfo.contact().c_str());
    pi.setUserId(plantInfo.has_manager() ? plantInfo.manager().c_str() : m_strUserId);
}

void VGNetManager::loggin()
{
    if (connectServer())
    {
        logginServer();
        _autoLoggin(true);
    }
}

void VGNetManager::disConnect()
{
    m_tcpClient->DisconnectTcp();
    setConnectState(2);
    _autoLoggin(false);
}

int VGNetManager::GetConnectState() const
{
    return m_connectState;
}

QString VGNetManager::GetUserId() const
{
    return m_strUserId;
}

void VGNetManager::sltTcpResult(bool connected)
{
    if (!connected)
        setConnectState(2);
}

void VGNetManager::sltAckInfo(const QString &name, const QByteArray &byte)
{
    if (name == d_p_ClassName(AckHeartBeat))
        m_nHeartTimes = 0;                  //心跳
    else if(name == d_p_ClassName(AckGSIdentityAuthentication))
        _processLoggin(byte);               //登陆结果
    else if (name == d_p_ClassName(AckDeleteParcelDescription))
        _processDelPDAck(byte);             //删除地块
    else if(name == d_p_ClassName(PostOperationInformation))
        _processGps(byte);                  //GPS 位置信息
    else if(name == d_p_ClassName(AckPostParcelDescription))
        _processUploadSurveyResult(byte);   //上传测绘信息结果
    else if(name == d_p_ClassName(AckRequestParcelDescriptions))
		_processSurveyLands(byte);          //地块描述信息
    else if(name == d_p_ClassName(AckPostOperationDescription))
        _processUploadOperationPlan(byte);   //上传规划结果
    else if(name == d_p_ClassName(AckRequestOperationDescriptions))
        _processOparetionPlan(byte);        //下载规划
    else if(name == d_p_ClassName(AckPostOperationRoute))
        _processUploadMissionResult(byte); //上传航线结果
    else if(name == d_p_ClassName(SyscOperationRoutes))
        _processSysAck(byte);               //同步航线结果
    else if(name == d_p_ClassName(AckRequestUavStatus))
        _processPlantState(byte);           //查询飞机状态结果
    else if(name == d_p_ClassName(AckRequestBindUav))
        _processPlantBind(byte);            //请求绑定、解除绑定应答
    else if (name == d_p_ClassName(AckRequestUavProductInfos))
        _processPlantInfo(byte);            //查询飞机信息结果
    else if (name == d_p_ClassName(AckPostControl2Uav))
        _processAckControl2Uav(byte);
    else if (name == d_p_ClassName(PostStatus2GroundStation))
        _processUav2Gc(byte);
    else if (name == d_p_ClassName(AckNewGS))
        _processAckNewGS(byte);

    m_lastRevTime = QTime::currentTime();
    m_nHeartTimes = 0;
}

void VGNetManager::sltHeartTimeout()
{
    if(m_tcpClient)
    {
        double time = 0;
        if(time < HEART_START_TIME)
        {
            QTime endTime = QTime::currentTime();
            time = m_lastRevTime.secsTo(endTime);
        }

        if(time > HEART_START_TIME && m_connectState == 1)
        {
            if(m_nHeartTimes < HEART_TIMEOUT_COUNT)
            {
                PostHeartBeat cmdPhb;
                _checSeq();
                cmdPhb.Clear();
                cmdPhb.set_seqno(m_seqno);
                cmdPhb.set_id(m_strUserId.toUtf8().data());
                _sendNetWork(cmdPhb);
                m_nHeartTimes++;
            }
            else
            {
                m_heartTimer.stop();
                m_nHeartTimes = 0;
                disConnect();
                _autoLoggin(true);
            }
        }
    }
}

void VGNetManager::_safeDeleteBoundary(const VGLandBoundary &bdr)
{
    DeleteParcelDescription sCmdDPD;
    if (m_connectState != 1)
        return;

    _checSeq();
    sCmdDPD.set_seqno(m_seqno);

    QString actId = bdr.GetLandActId();
    if (!actId.isEmpty())
    {
        sCmdDPD.set_pdid(actId.toUtf8().data());
        sCmdDPD.set_delpsi(true);
        sCmdDPD.set_delpc(true);
        _sendNetWork(sCmdDPD);
    }
}

void VGNetManager::_safeDeleteFlyRoute(const VGFlyRoute &fr)
{
    DeleteOperationDescription sDelOp;
    if (m_connectState != 1)
        return;

    QString actId = fr.GetActId();
    if (!actId.isEmpty())
    {
        _checSeq();
        sDelOp.set_seqno(m_seqno);
        sDelOp.set_odid(actId.toUtf8().data());
        _sendNetWork(sDelOp);
    }
}

int VGNetManager::_getSendNo(const QString &name) const
{
    QMap<QString, int>::const_iterator itr = m_mapSendSeqs.find(name);
    if (itr != m_mapSendSeqs.end())
        return itr.value();

    return -1;
}

void VGNetManager::_setSendNo(const QString &name, int seq)
{
    m_mapSendSeqs[name] = seq;
}

das::proto::OperationPlan *VGNetManager::_transOperationPlan(const VGFlyRoute &fr)
{
    if (OperationPlan *operationPlan = new OperationPlan())
    {
        operationPlan->set_width(fr.GetSprinkleWidth());
        operationPlan->set_angle(fr.GetAngle());
        operationPlan->set_anti(fr.GetAnti());
        operationPlan->set_single(fr.GetSingleShrink());
        for (int bdr : fr.GetEdgeShinkLs())
        {
            SingleShrink *sb = operationPlan->add_bdrsafe();
            sb->set_bdr(bdr);
            sb->set_safe(fr.GetEdgeShink(bdr));
        }
        operationPlan->set_block(fr.GetBlockSafeDis());
        return operationPlan;
    }

    return NULL;
}

void VGNetManager::registerSuc()
{
    qvgApp->SetQmlTip(QString::fromStdWString(L"注册成功"));
    disConnect();
    loggin();
    emit registed();
    emit connectChanged();
}

void VGNetManager::uploadSurveyInfo(const VGLandBoundary &desInfo)
{
    VGLandInformation *land = desInfo.GetBelongedLand();
    if (m_connectState != 1 || !land)
        return;

    ParcelDescription* pDes = new ParcelDescription();
    pDes->set_name(land->GetAddress().toUtf8().data());
    pDes->set_registerid(m_strUserId.toUtf8().data());
    pDes->set_acreage(desInfo.GetLandArea());
    if (!land->actualSurveyId().isEmpty())
        pDes->set_psiid(land->actualSurveyId().toUtf8().data());

    ParcelContracter* owner = new ParcelContracter();
    owner->set_name(land->ownerName().toUtf8().data());
    owner->set_address(land->ownerAddr().toUtf8().data());
    owner->set_birthdate(land->m_time);
    owner->set_mobileno(land->ownerPhone().toUtf8().data());
    pDes->set_allocated_pc(owner);

    QGeoCoordinate coor = land->GetCheckCoord();
    if (coor.isValid())
    {
        Coordinate* onecoordiante = new Coordinate();
        onecoordiante->set_latitude((qint32)(coor.latitude()*INTCOORLATORLONPOW));
        onecoordiante->set_longitude((qint32)(coor.longitude()*INTCOORLATORLONPOW));
        onecoordiante->set_altitude((qint32)(coor.altitude()*1000));
        pDes->set_allocated_coordinate(onecoordiante);
    }

    ParcelSurveyInformation* psi = _boundaryToParcelSurvey(&desInfo);
    if (!psi)
    {
        delete pDes;
        return;
    }
    pDes->set_allocated_psi(psi);
    PostParcelDescription m_cmdPpdes;
    _checSeq();
    m_cmdPpdes.set_seqno(m_seqno);
    m_cmdPpdes.set_allocated_pd(pDes);
    _sendNetWork(m_cmdPpdes);
    VGGlobalFunc::initialItemDescribe(m_mapDesUpload, desInfo, false);
}

void VGNetManager::uploadPlanningRoute(const VGFlyRoute &fr)
{
    if (m_connectState != 1 || !fr.IsValide())
        return;

    PostOperationDescription cmdPor;
    _checSeq();
    cmdPor.set_seqno(m_seqno);
    if (OperationDescription *od = new OperationDescription)
    {
        od->set_registerid(m_strUserId.toUtf8().data());
        QString strID = fr.GetBoundaryID();
        if (!strID.isEmpty())
            od->set_pdid(strID.toUtf8().data());
        if (!fr.m_strActId.isEmpty())
            od->set_odid(fr.m_strActId.toUtf8().data());

        od->set_crop(fr.GetCropper().toUtf8().data());
        od->set_drug(fr.GetPesticide().toUtf8().data());
        od->set_prize(fr.m_fPrise);
        if (!fr.m_strDescribe.isEmpty())
            od->set_notes(fr.m_strDescribe.toUtf8().data());
        od->set_plantime(fr.m_time);
        od->set_allocated_op(_transOperationPlan(fr));

        cmdPor.set_allocated_od(od);
    }

    _sendNetWork(cmdPor);
    VGGlobalFunc::initialItemDescribe(m_mapDesUpload, fr, false);
}

void VGNetManager::requestLandinfo(const QString &userId, const DescribeMap &dcs)
{
    if (m_connectState != 1)
        return;
    RequestParcelDescriptions cmdRPDess;
    _checSeq();
    cmdRPDess.set_seqno(m_seqno);
    if (!userId.isEmpty())
        cmdRPDess.set_registerid(userId.toUtf8().data());

    DescribeMap::const_iterator itr = dcs.find("surveryorname");
    if (itr != dcs.end())
        cmdRPDess.set_surveryorname(itr.value().toString().toUtf8().data());

    itr = dcs.find("surveryorid");
    if (itr != dcs.end())
        cmdRPDess.set_surveryorid(itr.value().toString().toUtf8().data());

    itr = dcs.find("parcelname");
    if (itr != dcs.end())
        cmdRPDess.set_parcelname(itr.value().toString().toUtf8().data());

    itr = dcs.find("contracter");
    if (itr != dcs.end())
        cmdRPDess.set_contracter(itr.value().toString().toUtf8().data());

    itr = dcs.find("mobileno");
    if (itr != dcs.end())
        cmdRPDess.set_mobileno(itr.value().toString().toUtf8().data());

    itr = dcs.find("zones");
    if (itr != dcs.end())
    {
        QList<QGeoCoordinate> lsCoor = itr.value().value<QList<QGeoCoordinate> >();
        foreach (const QGeoCoordinate &coordinate, lsCoor)
        {
            Coordinate* coordinateRect = new Coordinate();
            coordinateRect->set_latitude(coordinate.latitude() * INTCOORLATORLONPOW);
            coordinateRect->set_longitude(coordinate.longitude() * INTCOORLATORLONPOW);
            coordinateRect->set_altitude(coordinate.altitude() * 1e3);

            Coordinate* coordinate2 = cmdRPDess.add_zones();
            *coordinate2 = *coordinateRect;
        }
    }
    _sendNetWork(cmdRPDess);
}

void VGNetManager::DeleteItemSafe(const MapAbstractItem *item)
{
    if (!item)
        return;

    if (item->ItemType() == MapAbstractItem::Type_LandBoundary)
        _safeDeleteBoundary(*(const VGLandBoundary*)item);
    if (item->ItemType() == MapAbstractItem::Type_FlyRoute)
        _safeDeleteFlyRoute(*(const VGFlyRoute*)item);
}

void VGNetManager::requestOperationPlan(const QString &userId, const DescribeMap &dcs)
{
    if (m_connectState != 1)
        return;
    _checSeq();

    RequestOperationDescriptions cmdRODess;
    cmdRODess.set_seqno(m_seqno);

    if (!userId.isEmpty())
        cmdRODess.set_registerid(userId.toUtf8().data());

    DescribeMap::const_iterator itr = dcs.find("odid");
    if (itr != dcs.end())
        cmdRODess.set_odid(itr.value().toString().toUtf8().data());

    itr = dcs.find("pdid");
    if (itr != dcs.end())
        cmdRODess.set_pdid(itr.value().toString().toUtf8().data());

    _sendNetWork(cmdRODess);
}

void VGNetManager::requestPlanning(const QString &pdid, const QString &odid)
{
    if (m_connectState != 1)
        return;
    RequestOperationDescriptions cmdROR;
    _checSeq();
    cmdROR.set_seqno(m_seqno);

    if (!pdid.isEmpty())
        cmdROR.set_pdid(pdid.toUtf8().data());
    else if (!odid.isEmpty())
        cmdROR.set_odid(odid.toUtf8().data());
    else
        cmdROR.set_registerid(m_strUserId.toUtf8().data());

    _sendNetWork(cmdROR);
}

void VGNetManager::_processUploadMissionResult(const QByteArray &byte)
{
    static AckPostOperationRoute sAckPor;               //上传航线规划应答
    if (sAckPor.ParseFromArray(byte.data(), byte.size()))
    {
        if (sAckPor.result() == 1)
            emit missionUploaded(m_mapDesUpload["CurrentPlan"].toString());
    }
}

void VGNetManager::syncPlanning(VGVehicleMission *ms, const QString &uavId)
{
    VGFlyRoute *fr = ms ? ms->GetFlyRoute() : NULL;
    if (m_connectState != 1 || !fr || !fr->IsValide())
        return;

    PostOperationRoute cmdPor;
    _checSeq();
    cmdPor.set_seqno(m_seqno);

    OperationRoute *operationRoute = new OperationRoute();
    operationRoute->set_gsid(fr->GetUser().toUtf8().data());
    if (!fr->m_strActId.isEmpty())
        operationRoute->set_rpid(fr->m_strActId.toUtf8().data());
    operationRoute->set_uavid(uavId.toUtf8().data());
    operationRoute->set_createtime(fr->GetTime());
    operationRoute->set_maxvoyage(fr->GetMaxRoute());

    //航点列表
    for (MissionItem *item : ms->MissionItems())
    {
        mavlink_message_t message;
        VGMavLinkCode::EncodeMissionItemInt(message, *item, item->sequenceNumber() == 0);
        operationRoute->add_missions(message.payload64, MAVLINK_MSG_ID_MISSION_ITEM_INT_LEN);
    }
    if (fr->CountBlock() > 0)
    {
        for (MissionItem *item : ms->BoundaryItems())
        {
            mavlink_message_t message;
            VGMavLinkCode::EncodeMissionItemInt(message, *item, item->sequenceNumber() == 0);
            operationRoute->add_boundarys(message.payload64, MAVLINK_MSG_ID_MISSION_ITEM_INT_LEN);
        }
    }

    m_mapDesUpload["CurrentPlan"] = uavId;
    cmdPor.set_allocated_or_(operationRoute);
    _sendNetWork(cmdPor);
}

void VGNetManager::SendControlUav(const QString &uav, const mavlink_message_t &mav)
{
    if (mav.len > 64 || m_connectState != 1)
        return;

    PostControl2Uav gc2uav;
    _checSeq();
    gc2uav.set_seqno(m_seqno);
    gc2uav.set_uavid(uav.toUtf8().data());
    gc2uav.set_userid(m_strUserId.toUtf8().data());

    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    *(uint16_t*)buffer = mav.msgid;
    int len = mav.len+sizeof(uint16_t);
    memcpy(buffer + sizeof(uint16_t), mav.payload64, mav.len);
    gc2uav.add_data(buffer, len);

    _sendNetWork(gc2uav);
}

void VGNetManager::SendControlUav(const QString &uav, const QList<MissionItem*> &items)
{
    if (items.count() <= 0 || m_connectState != 1)
        return;

    PostControl2Uav gc2uav;
    _checSeq();
    gc2uav.set_seqno(m_seqno);
    gc2uav.set_uavid(uav.toUtf8().data());
    gc2uav.set_userid(m_strUserId.toUtf8().data());
    mavlink_message_t message;
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN + 2];
    for (MissionItem *item : items)
    {
        VGMavLinkCode::EncodeMissionItemInt(message, *item, item->sequenceNumber() == 0);
        *(uint16_t*)buffer = message.msgid;
        memcpy(buffer + sizeof(uint16_t), message.payload64, message.len);
        gc2uav.add_data(buffer, message.len+2);
    }
    _sendNetWork(gc2uav);
}

void VGNetManager::SendControlUav(const QString &uav, const QMap<QString, QVariant> &params)
{
    if (params.count() <= 0 || m_connectState != 1)
        return;

    PostControl2Uav gc2uav;
    _checSeq();
    gc2uav.set_seqno(m_seqno);
    gc2uav.set_uavid(uav.toUtf8().data());
    gc2uav.set_userid(m_strUserId.toUtf8().data());
    mavlink_message_t message;
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN + 2];
    int nLen = 70;
    QMap<QString, QVariant>::const_iterator itr = params.begin();
    for (; itr!= params.end(); ++itr)
    {
        if (!itr.value().isValid())
            VGMavLinkCode::EncodeGetParameter(message, itr.key());
        else
            VGMavLinkCode::EncodeSetParameter(message, itr.key(), itr.value());

        *(uint16_t*)buffer = message.msgid;
        memcpy(buffer + sizeof(uint16_t), message.payload64, message.len);
        gc2uav.add_data(buffer, message.len + 2);
        nLen += message.len + 2;
        if(nLen > 200)
            break;
    }
    _sendNetWork(gc2uav);
}

void VGNetManager::requestUavInfo(const QStringList &lstPlaneId)
{
    if(m_connectState != 1 && lstPlaneId.size() <= 0)
        return;

    RequestUavStatus cmdRUS;
    _checSeq();
    cmdRUS.set_seqno(m_seqno);

    foreach(const QString &plantId, lstPlaneId)
    {
        if (plantId.isEmpty())
            continue;

        cmdRUS.add_uavid(plantId.toUtf8().data());
    }

    _sendNetWork(cmdRUS);
}

void VGNetManager::requestUavInfo(const QGeoCoordinate &/*coor*/, double /*dis*/)
{
}

void VGNetManager::requestUavStatus(const QString &plantId)
{
    if(m_connectState != 1 || plantId.isEmpty())
        return;

    RequestUavStatus cmdRUS;
    _checSeq();
    cmdRUS.set_seqno(m_seqno);
    cmdRUS.add_uavid(plantId.toUtf8().data());
    _sendNetWork(cmdRUS);
}

void VGNetManager::requestBindUav(const QString &uavId, int type)
{
    if (m_connectState != 1 || uavId.isEmpty())
        return;

    RequestBindUav cmdRBS;
    _checSeq();
    cmdRBS.set_seqno(m_seqno);

    if(!uavId.isEmpty())
    {
        cmdRBS.set_uavid(uavId.toUtf8().data());
        cmdRBS.set_opid(type);
        cmdRBS.set_binder(m_strUserId.toUtf8().data());
    }
    else
    {
        return;
    }

    _sendNetWork(cmdRBS);
}

bool VGNetManager::UploadItem(const MapAbstractItem *item)
{
    if (!item)
        return false;

    m_mapDesUpload.clear();
    if (item->ItemType() == MapAbstractItem::Type_LandBoundary)
        uploadSurveyInfo(*(const VGLandBoundary*)item);
    else if (item->ItemType() == MapAbstractItem::Type_FlyRoute)
        uploadPlanningRoute(*(const VGFlyRoute*)item);
    else
        return false;

    return true;
}

void VGNetManager::setDirectLinkUav(const QString &uavId)
{
    if(uavId.length() == 0)
        return;

    RequestUavProductInfos cmdRUPI;
    _checSeq();
    cmdRUPI.set_seqno(m_seqno);
    cmdRUPI.add_uavid(uavId.toUtf8().data());

    _sendNetWork(cmdRUPI);
}

void VGNetManager::_autoLoggin(bool bAuto)
{
    if (bAuto && m_autoLoggin)
    {
        if (m_tmAutoLoggin > 0)
            return;

        m_tmAutoLoggin = startTimer(3000);
        QTimerEvent e(m_tmAutoLoggin);
        timerEvent(&e);
    }
    else
    {
        if (m_tmAutoLoggin < 0)
            return;

        killTimer(m_tmAutoLoggin);
        m_tmAutoLoggin = -1;
    }
}

void VGNetManager::_checSeq()
{
    m_seqno++;
    if (!m_seqno)
        m_seqno = 1;
}

void VGNetManager::_sendNetWork(const ::google::protobuf::MessageLite &msg)
{
    int msglen = msg.ByteSize();
    QString name = msg.GetTypeName().c_str();
    QByteArray ay(msglen, 0);
    msg.SerializeToArray(ay.data(), msglen);
    emit sigSendRequestInfo(name, ay);
    _setSendNo(name, m_seqno);
}

ParcelSurveyInformation *VGNetManager::_boundaryToParcelSurvey(const VGLandBoundary *bdr)
{
    if (!bdr)
        return NULL;
    ParcelSurveyInformation *psi(new ParcelSurveyInformation());
    if (!psi)
        return NULL;

    psi->set_surverytime(bdr->GetTime());
    psi->set_surveryorid(bdr->GetUser().toUtf8().data());
    psi->set_sp(transform2Precision(bdr->GetSurveyType()));

    if (!bdr->m_cloudID.isEmpty())
        psi->set_id(bdr->m_cloudID.toUtf8().data());

    foreach(VGLandPolygon *ply, bdr->Polygos())
    {
        if (ply->GetId() == VGLandPolygon::Boundary)
        {
            Outline *outline(new Outline);
            outline = _polygonToOutline(outline, *ply);
            psi->set_allocated_po(outline);
            if (!outline || outline->coordinates_size() < 2)
            {
                delete psi;
                return NULL;
            }
        }
        else if (ply->GetId() == VGLandPolygon::BlockBoundary)
        {
            Outline *outlineBlock = _polygonToOutline(psi->add_bo(), *ply);
            if (!outlineBlock)
            {
                delete psi;
                return NULL;
            }
        }
    }

    return psi;
}

Outline *VGNetManager::_polygonToOutline(Outline *outline, const VGLandPolygon &plg)
{
    if (!outline)
        return NULL;

    foreach (VGCoordinate *itr, plg.GetCoordinates())
    {
        QGeoCoordinate coordinate = itr->GetCoordinate();
        Coordinate *coordinateOutline = outline->add_coordinates();
        coordinateOutline->set_altitude((int)(coordinate.altitude() * 1000));
        coordinateOutline->set_latitude((qint32)(coordinate.latitude() * INTCOORLATORLONPOW));
        coordinateOutline->set_longitude((qint32)(coordinate.longitude() * INTCOORLATORLONPOW));
    }
    return outline;
}

VGLandInformation *VGNetManager::_parseLand(const ParcelDescription &pDes)
{
    VGLandInformation *land = new VGLandInformation(this);
    if (!land)
        return NULL;

    land->m_actualSurveyId = pDes.id().c_str();
    if (!m_bInitial)
        m_strPdids << land->m_actualSurveyId;

    const ParcelSurveyInformation &psi = pDes.psi();
    land->m_adress = pDes.name().c_str();
    if (pDes.has_coordinate())
    {
        const Coordinate &c = pDes.coordinate();
        double lat = double(c.latitude()) / INTCOORLATORLONPOW;
        double lon = double(c.longitude()) / INTCOORLATORLONPOW;
        double altitude = double(c.altitude()) / 1e3;
        land->SetCheckCoord(QGeoCoordinate(lat, lon, altitude));
    }
    land->m_typeSurvey = transform2SurveyType(psi.sp());
    const ParcelContracter &owner = pDes.pc();
    land->m_owner.strName = owner.name().c_str();
    land->m_owner.strAddress = owner.address().c_str();
    land->m_owner.strPhone = owner.mobileno().c_str();
    land->m_time = owner.birthdate();

    land->m_userId = psi.surveryorid().c_str();
    land->m_bUploaded = true;
    return land;
}

VGLandBoundary *VGNetManager::_parseBoundary(const ParcelSurveyInformation &psi)
{
    VGLandBoundary *bdr = new VGLandBoundary(this);
    if (!bdr)
        return NULL;

    bdr->m_cloudID = psi.id().c_str();
    bdr->m_time = psi.surverytime(); 

    VGLandPolygon *plg = bdr->_addBoundary(VGLandPolygon::Boundary);
    _toPolygon(psi.po(), *plg);
    int nBlockCount = psi.bo_size();
    
    for (int i = 0; i < nBlockCount; i++)
    {
        VGLandPolygon *plgTmp = bdr->_addBoundary(VGLandPolygon::BlockBoundary);
        _toPolygon(psi.bo(i), *plgTmp);
    }
    bdr->m_bUploaded = true;
    return bdr;
}

VGFlyRoute *VGNetManager::_parseFlyRoute(const das::proto::OperationDescription &od, DescribeMap &dsc)
{
    if (VGFlyRoute *fr = new VGFlyRoute())
    {
        dsc["boundaryId"] = od.pdid().c_str();
        fr->m_bUploaded = true;
        const OperationPlan &op = od.op();
        fr->m_sprinkleWidth = op.width();
        fr->m_fAngle = op.angle();
        fr->m_bAnti = op.anti();
        fr->m_bSingleShrink = op.single();
        for (int i = 0; i < op.bdrsafe_size(); ++i)
        {
            const SingleShrink &ss = op.bdrsafe(i);
            if (ss.bdr() < 0)
                fr->m_fSafeDis = ss.safe();
            else
                fr->m_lsOutlineSafe.push_back(VGFlyRoute::EdgeShrinkPair(ss.bdr(), ss.safe()));
        }
        fr->m_fBlockSafeDis = op.block();

        fr->m_strActId = od.odid().c_str();
        if (od.has_crop())
            fr->m_strCropper = od.crop().c_str();
        if (od.has_drug())
            fr->m_strPesticide = od.drug().c_str();
        fr->m_fPrise = od.prize();
        if (od.has_notes())
            fr->m_strDescribe = od.notes().c_str();

        fr->m_time = od.plantime();
        return fr;
    }
    return NULL;
}

void VGNetManager::_parseOperationRoute(const OperationRoute &route)
{
}
