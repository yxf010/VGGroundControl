#include "VGQXManager.h"
#include "VGApplication.h"
#include <QSettings>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include "VGLandManager.h"
#include "VGGlobalFun.h"
#include "VGSurveyMonitor.h"
#include "VGMapManager.h"

#define QXPORT 8002
#define QXGGASPACE 2000
#define TMOUTRTCM 10000
#define QXWZLOGINOK "ICY 200 OK"
#define QXHOSTNAME "rtk.ntrip.qxwz.com/RTCM32_GGB"

static bool isUsefulRTCM(uint16_t type)
{
    return type == VGQXManager::TYPE_STATION
        || type == VGQXManager::TYPE_GPS
        || type == VGQXManager::TYPE_HALILEO
        || type == VGQXManager::TYPE_QZSS
        || type == VGQXManager::TYPE_BEIDOU;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//VGQXManager
//////////////////////////////////////////////////////////////////////////////////////////////////////
VGQXManager::VGQXManager(QObject *parent /*= NULL*/): QObject(parent)
, m_bStart(false), m_socket(NULL), m_tmLastRTCM(QDateTime::currentMSecsSinceEpoch())
, m_thread(new QThread), m_bAcceptGGA(false), m_idTimer(-1), m_bRtcm(false)
, m_tmLastGGA(-1)
{
    connect(this, &VGQXManager::startedChanged, this, &VGQXManager::onStartChanged);
    connect(this, &VGQXManager::writeGGA, this, &VGQXManager::onWriteGGA);
    loadConfig();
    if (m_thread)
        moveToThread(m_thread);

    m_thread->start();
}

VGQXManager::~VGQXManager()
{
}

void VGQXManager::SendGGA(const QByteArray &gga)
{
    if (!m_bAcceptGGA || QDateTime::currentMSecsSinceEpoch()- m_tmLastGGA >QXGGASPACE)
        emit writeGGA(gga);
}

bool VGQXManager::GetRtcm() const
{
    return m_bRtcm;
}

QByteArray VGQXManager::GenerateGGa(const QGeoCoordinate &coor, double altRel, int sat, double pr)
{
    double lat = VGSurveyMonitor::toGGAValue(coor.latitude());
    double lon = VGSurveyMonitor::toGGAValue(coor.longitude());
    double alt = coor.altitude();
    QByteArray array = QString("$GPGGA,%1,%2,%3,%4,%5,1,%6,%7,%8,M,%9,M,,").arg(QDateTime::currentDateTimeUtc().toString("hhmmss.z"))
        .arg(lat, 0, 'f', 8).arg(lat > 0 ? 'N' : 'S').arg(lon, 0, 'f', 8).arg(lon > 0 ? 'E' : 'W')
        .arg(sat).arg(pr).arg(coor.altitude(), 0, 'f', 3).arg(altRel-alt, 0, 'f', 3).toUtf8();

    uint8_t sum = 0;
    for (int i = 1; i < array.size(); ++i)
        sum ^= array[i];
    array += QString("*%1\r\n").arg(QString::number(sum, 16)).toUpper().toUtf8();
    return array;
}

void VGQXManager::SetUrl(const QString &url)
{
    if (m_strAcc == url)
        return;

    m_strUrl = url;
    emit urlChanged(url);
}

QString VGQXManager::GetUrl() const
{
    return m_strUrl;
}

void VGQXManager::SetPort(quint16 p)
{
    if (m_port == p)
        return;

    m_port = p;
    emit portChanged(p);
}

quint16 VGQXManager::GetPort() const
{
    return m_port;
}

void VGQXManager::SetAccount(const QString &acc)
{
    if (m_strAcc == acc)
        return;

    m_strAcc = acc;
    emit accChanged(acc);
}

QString VGQXManager::GetAccount() const
{
    return m_strAcc;
}

void VGQXManager::SetPassword(const QString &pswd)
{
    if (m_strPswd == pswd)
        return;
    m_strPswd = pswd;
    emit pswdChanged(pswd);
}

QString VGQXManager::GetPassword() const
{
    return m_strPswd;
}

void VGQXManager::SetStart(bool b)
{
    if (b == m_bStart)
        return;

    m_bStart = b;
    emit startedChanged(b);
}

bool VGQXManager::GetStart() const
{
    return m_bStart;
}

void VGQXManager::saveConfig()
{
    if (QSettings *settings = qvgApp->GetSettings())
    {
        settings->beginGroup("QX");
        settings->setValue("URL", m_strUrl);
        settings->setValue("Port", m_port);
        settings->setValue("account", m_strAcc);
        settings->setValue("password", m_strPswd);
        settings->endGroup();
    }
}

void VGQXManager::loadConfig()
{
    if (QSettings *settings = qvgApp->GetSettings())
    {
        settings->beginGroup("QX");
        m_strUrl = settings->value("URL", QXHOSTNAME).toString();
        m_port = settings->value("Port", QXPORT).toUInt();
        m_strAcc = settings->value("account").toString();
        m_strPswd = settings->value("password").toString();
        settings->endGroup();
    }
}

void VGQXManager::SetRtcm(bool b)
{
    if (b)
        m_tmLastRTCM = QDateTime::currentMSecsSinceEpoch();

    if (m_bRtcm == b)
        return;

    m_bRtcm = b;
    emit rtcmChanged(b);
}

void VGQXManager::timerEvent(QTimerEvent *event)
{
    if (m_idTimer == event->timerId())
    {
        qint64 tm = QDateTime::currentMSecsSinceEpoch() - m_tmLastRTCM;
        if (tm > TMOUTRTCM && m_socket)
            m_socket->close();

        if (m_bAcceptGGA && tm > 2 * QXGGASPACE)
            SetRtcm(false);
        return;
    }
    QObject::timerEvent(event);
}

void VGQXManager::onStartChanged(bool b)
{
    if (m_strUrl.isEmpty() || m_strUrl.front() == "/")
        return;

    if (b && !m_strAcc.isEmpty() && !m_strPswd.isEmpty())
    {
        m_socket = new QTcpSocket(this);
        connect(m_socket, &QTcpSocket::connected, this, &VGQXManager::onConneted);
        connect(m_socket, &QTcpSocket::disconnected, this, &VGQXManager::onDisconnected);
        connect(m_socket, &QTcpSocket::readyRead, this, &VGQXManager::onReadyRead);
        int pos = m_strUrl.indexOf("/");
        QString host = pos >= 0 ? m_strUrl.left(pos) : m_strUrl;
        m_socket->connectToHost(host, m_port);
        m_tmLastRTCM = QDateTime::currentMSecsSinceEpoch();
    }
    else if (!b && m_socket)
    {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = NULL;
        m_bAcceptGGA = false;
    }
}

void VGQXManager::onReadyRead()
{
    while (m_socket->bytesAvailable() > 0)
    {
        m_arrayRecv += m_socket->readAll();
    }

    if (!m_bAcceptGGA && m_arrayRecv.contains(QXWZLOGINOK))
    {
        m_bAcceptGGA = true;
        m_arrayRecv.clear();
        QTimer::singleShot(100, this, &VGQXManager::onServerReady);
    }
    else if (m_bAcceptGGA)
    {
        while (true)
        {
            int ps = _parseRtcm(m_arrayRecv);
            if (ps > 0)
                m_arrayRecv.remove(0, ps);
            else
                break;
        }
    }
}

void VGQXManager::onConneted()
{
    if (m_socket)
    {
        int pos = m_strUrl.indexOf("/");
        _beginGet(pos>=0? m_strUrl.mid(pos):QString());
        _addEle("User-Agent", "NTRIP cskdby3006");
        _addEle("Accept", "*");
        _addEle("Connection", "close");
        _addAuthorization(m_strAcc, m_strPswd);
        _endGet();
        m_arrayRecv.clear();

        m_tmLastRTCM = QDateTime::currentMSecsSinceEpoch();
        if (m_idTimer < 0)
            m_idTimer = startTimer(1000);
    }
}

void VGQXManager::onDisconnected()
{
    m_socket->deleteLater();
    m_socket = NULL;
    m_bAcceptGGA = false;
    if (m_idTimer >= 0)
    {
        killTimer(m_idTimer);
        m_idTimer = -1;
    }
    m_arrayGGa.clear();
    m_bStart = false;
}

void VGQXManager::onWriteGGA(const QByteArray &gga)
{
    if (!m_bStart)
    {
        SetStart(true);
        m_arrayGGa = gga;
        return;
    }

    _writeGGa(gga);
}

void VGQXManager::onServerReady()
{
    if (m_arrayGGa.size() <= 0)
        m_arrayGGa = GenerateGGa(qvgApp->landManager()->GetCurCoordinate(), 0, qvgApp->landManager()->GetNumbSatlate());

    _writeGGa(m_arrayGGa);
}

void VGQXManager::_writeGGa(const QByteArray &gga)
{
    if (gga.size() > 0 && m_bAcceptGGA)
    {
        m_socket->write(gga);
        m_tmLastGGA = QDateTime::currentMSecsSinceEpoch();
    }
}

void VGQXManager::_beginGet(const QString &url)
{
    if (url.size()>0 && url.front() == '/')
        m_arraySend = QString("GET %1 HTTP/1.0\r\n").arg(url).toUtf8().data();
    else
        m_arraySend = QString("GET /%1 HTTP/1.0\r\n").arg(url).toUtf8().data();
}

void VGQXManager::_addEle(const QString &key, const QByteArray &value)
{
    m_arraySend += key.toUtf8() + ": " + value + "\r\n";
}

void VGQXManager::_addAuthorization(const QString &acc, const QString &pswd)
{
    QByteArray arr = QByteArray("Basic ") + QString("%1:%2").arg(acc).arg(pswd).toUtf8().toBase64();
    _addEle("Authorization", arr);
}

void VGQXManager::_endGet()
{
    m_arraySend += "\r\n";
    if (m_socket)
        m_socket->write(m_arraySend);

    m_arraySend.clear();
}

int VGQXManager::_parseRtcm(const QByteArray &value)
{
    QList<uint16_t> idxs;
    QList<QByteArray> rtcms;
    for (int i = 0; i < value.size() - 3; ++i)
    {
        if (uint8_t(value[i]) == FRAME_HEADER)
        {
            uint16_t len = (VGGlobalFunc::bytesToUShort(value.mid(i + 1, 2)) & 0x03ff) + 6;
            if (len <= value.size()-i && len<512)
            {
                uint16_t type = ((VGGlobalFunc::bytesToUShort(value.mid(i+3, 2))>>4) & 0x0fff);
                SetRtcm(true);
                bool bRTCMPack = idxs.contains(type);
                if (!isUsefulRTCM(type))
                {
                    i += len - 1;
                    continue;
                }
                if (!bRTCMPack)
                {
                    idxs << type;
                    rtcms << value.mid(i, len);
                }

                if (bRTCMPack || (idxs.size() > 1 && i + len == value.size()))
                {
                    emit rtcmRead(idxs, rtcms);
                    return bRTCMPack ? i : i + len;
                }

                i += len - 1;
            }
        }
    }
    return 0;
}
