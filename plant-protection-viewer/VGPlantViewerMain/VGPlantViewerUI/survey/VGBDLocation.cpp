#include "VGBDLocation.h"
#include <QDateTime>
#include <string.h>
#include <QGeoCoordinate>
#include "VGSurveyMonitor.h"
#include "VGApplication.h"
#include "VGGlobalFun.h"
#include "VGQXManager.h"
#include "LinkInterface.h"
#include "VGBluetoothManager.h"

#define  TMOUTLink 5000
#define  TMSPACEGGA 5000

enum {
    PACKFRAM_SIZE = 19,
    PACKHEADER = 0x7e,
    PACKTAILER = 0x7a,

    PACKTYPE_INIT = 1,
    PACKTYPE_REREQ = 2,
    PACKTYPE_INITACK = 6,
    PACKTYPE_POW = 7,
    PACKTYPE_FINISHFRAM = 8,
};
//////////////////////////////////////////////////////////////////////////////////
//VGBDLocation::BitList
//////////////////////////////////////////////////////////////////////////////////
VGBDLocation::BitList::BitList(uchar count) :m_count(count)
{
    memset(m_lstFrame, 0xff, sizeof(m_lstFrame));
    for (int i = 0; i < m_count; ++i)
    {
        SetValue(i, false);
    }
}

void VGBDLocation::BitList::SetSize(uchar count)
{
    if (m_count == count)
        return;

    m_count = count;
    for (int i = 0; i < m_count; ++i)
    {
        SetValue(i, false);
    }
}

int VGBDLocation::BitList::GetSize() const
{
    return m_count;
}

void VGBDLocation::BitList::SetValue(uchar index, bool b)
{
    if (index >= m_count)
        return;

    uchar &bBit = m_lstFrame[index / 8];
    if (b)
        bBit |= (1 << index % 8);
    else
        bBit &= ~(1 << index % 8);
}

bool VGBDLocation::BitList::GetValue(uchar index)const
{
    if (index >= m_count)
        return false;

    const uchar &bBit = m_lstFrame[index / 8];
    return (bBit & (1 << index % 8)) != 0;
}

int VGBDLocation::BitList::GetFirstFrom(bool b, uchar from /*= 0*/) const
{
    for (uchar i = from; i < m_count; ++i)
    {
        if (GetValue(i) == b)
            return i;
    }

    return -1;
}

int VGBDLocation::BitList::GetFirstTo(bool b, uchar to /*= 0*/) const
{
    uchar count = to > m_count ? m_count : to;
    for (uchar i = 0; i < count; ++i)
    {
        if (GetValue(i) == b)
            return i;
    }

    return -1;
}
//////////////////////////////////////////////////////////////////////
//VGBDLocation
//////////////////////////////////////////////////////////////////////
VGBDLocation::VGBDLocation(QObject *p) : QObject(p)
, m_lstFramStat(0), m_type(0), m_lenth(0), m_oth(0)
, m_bIdle(true), m_nFramWrite(-1), m_bConnect(false)
, m_tmLastRctm(QDateTime::currentMSecsSinceEpoch())
, m_tmLastGGA(0), m_idTimer(-1)
{
    connect(this, &VGBDLocation::_rtcmWrite, this, &VGBDLocation::onWtiteRTCM);
}

void VGBDLocation::Linked(LinkInterface *link)
{
    if (link)
    {
        connect(link, &LinkInterface::bytesReceived, this, &VGBDLocation::onReceive);
        connect(this, &VGBDLocation::_writeBlt, link, &LinkInterface::writeBytes);
    }
}

void VGBDLocation::SendRTCM(const QList<uint16_t> idxs, const QList<QByteArray> &rtcms)
{
    if (QDateTime::currentMSecsSinceEpoch() - m_tmLastRctm > 1000 && rtcms.size()>0)
    {
        QByteArray sendA;
        foreach(const QByteArray &a, rtcms)
        {
            sendA += a;
        }
        emit _rtcmWrite(sendA);
    }
}

void VGBDLocation::onReceive(LinkInterface *, const QByteArray &data)
{
    bool bPush = false;
    while (data.size() > 0)
    {
        if (m_lstFramStat.GetSize() > 0 && uint8_t(data[0]) < m_lstFramStat.GetSize())
        {
            uint8_t cmd = _checkFrame(data);
            if (cmd > 0)
                _sendAck(cmd);

            if (_isIdle() && !m_arrayRtcm.isEmpty() && m_nFramWrite<0)
                _sendAck(PACKTYPE_INIT);

            break;
        }
        else
        {
            if (!bPush)
                m_arrayRcv.push_back(data);

            bPush = true;
            if (!_checkPack())
                break;
        }
    }
}

void VGBDLocation::onWtiteRTCM(const QByteArray &data)
{
    if (m_arrayRtcm.isEmpty())
        m_arrayRtcm = data;

    if (_isIdle() && m_bConnect)
        _sendAck(PACKTYPE_INIT);
}

bool VGBDLocation::_isIdle()const
{
    return m_bIdle;
}

void VGBDLocation::_begWriteRtcm()
{
    if (_isIdle() && !m_arrayRtcm.isEmpty())
    {
        m_nFramWrite = 0;
        bool b = true;
        while (b)
        {
            b = _writeRtcm(m_nFramWrite++);
        }
    }
}

bool VGBDLocation::_writeRtcm(uchar frame)
{
    int pos = unsigned(PACKFRAM_SIZE) * frame;
    int len = m_arrayRtcm.size() - pos;
    if (len <= 0)
        return false;

    QByteArray arr = m_arrayRtcm.mid(pos, len > PACKFRAM_SIZE ? PACKFRAM_SIZE : len);
    arr.push_front(frame);
    _send(arr);
    return true;
}

uchar VGBDLocation::_checkFrame(const QByteArray &arr)
{
    uchar nFram = (uchar)arr.at(0);
    unsigned nSize = arr.size();
    if (m_lstFramStat.GetSize() <= 0 && nFram >= m_lstFramStat.GetSize())
        return 0;

    if (nSize < 2)
        return 0;

    m_lstFramStat.SetValue(nFram, true);
    memcpy(m_content.data() + PACKFRAM_SIZE * nFram, arr.mid(1).data(), nSize - 1);
    m_tmLstFrame = QDateTime::currentMSecsSinceEpoch();
    int nF = m_lstFramStat.GetFirstTo(false, m_lstFramStat.GetSize());
    if (nF >= 0 && nF <= nFram)
    {
        m_oth = nF;
        return 0x02;
    }
    if (nFram == m_lstFramStat.GetSize() - 1)
    {
        int nF = m_lstFramStat.GetFirstTo(false, nFram);
        if (nSize < 20)
            m_content[PACKFRAM_SIZE * nFram + nSize - 1] = '\0';

        if (nF < 0)
        {
            m_lstFramStat.SetSize(0);
            _parseNmea(m_content.data());
            m_bIdle = true;
            return 0x08;
        }
        m_oth = nF;
        return 0x02;
    }
    return 0xff;
}

bool VGBDLocation::_checkPack()
{
    int nParsed = 0;
    bool bRet = false;
    for (int idx = 0; idx < m_arrayRcv.size()+1; ++idx)
    {
        if (PACKHEADER == uint8_t(m_arrayRcv[idx]))
        {
            int lenPack = uint8_t(m_arrayRcv[idx+1]);
            if (idx + lenPack <= m_arrayRcv.size())
            {
                _parsePack(m_arrayRcv.mid(idx, lenPack));
                nParsed = idx + lenPack;
                bRet = true;
                break;
            }
        }
        else
        {
            nParsed = idx;
        }
    }

    if (nParsed > 0)
        m_arrayRcv.remove(0, nParsed);

    return bRet;
}

uchar VGBDLocation::_prcsPackInit(const QByteArray &arr)
{
    m_tmLastGGA = QDateTime::currentMSecsSinceEpoch();
    if (m_idTimer < 0)
        m_idTimer = startTimer(1000);
    onLinkConnect(true);
    if ((m_lstFramStat.GetFirstFrom(false) >= 0 && !_isTimeout()))
        return 0;
    m_lstFramStat.SetSize(uint8_t(arr[3]));
    m_content.resize(PACKFRAM_SIZE * uint8_t(arr[3]));
    m_bIdle = false;
    return PACKTYPE_INITACK;
}

void VGBDLocation::_parsePack(const QByteArray &arr)
{
    if (arr.size() < 4 || uint8_t(arr[0]) != PACKHEADER || uint8_t(arr[1]) != arr.size())
        return;

    uchar cmdAck = 0;
    switch (uint8_t(arr[2]))
    {
    case PACKTYPE_INIT:
        cmdAck = _prcsPackInit(arr);
        break;
    case PACKTYPE_REREQ:
        for (size_t i = 0; i < uint8_t(arr[3]); i++)
            _writeRtcm(uint8_t(arr[4+i]));
        break;
    case PACKTYPE_INITACK:
        _begWriteRtcm();
        break;
    case PACKTYPE_POW:
        _powPer(uint8_t(arr[3]));
        break;
    case PACKTYPE_FINISHFRAM:
        m_nFramWrite = -1;
        m_arrayRtcm.clear();
        break;
    }

    if (cmdAck > 0)
        _sendAck(cmdAck);
}

void VGBDLocation::_parseNmea(const QString &str)
{
    foreach(const QString &strItr, str.split("\r\n", QString::SkipEmptyParts))
    {
        QStringList strLs = strItr.split(',');
        if (strLs.size() < 0 || strLs.first().compare("$GPGGA"))
            continue;

        if (_parPos(strLs) > 0)
            qvgApp->qxManager()->SendGGA((strItr + "\r\n").toUtf8());
    }
}

void VGBDLocation::_powPer(uchar per)
{
    if (VGSurveyMonitor *mnt = qvgApp->surveyMonitor())
        mnt->SetPowerPercent(per);
}

int VGBDLocation::_parPos(const QStringList &strLs)
{
    int ret = -1;
    VGSurveyMonitor *mnt = qvgApp->surveyMonitor();
    if (!mnt)
        return ret;

    double lat = 361;
    int idx = strLs.indexOf(QRegExp("S|N"));
    if (idx > 0)
        lat = VGSurveyMonitor::correctValue(strLs.at(idx - 1).toDouble(), strLs.at(idx));

    double lon = 361;
    if ((idx = strLs.indexOf(QRegExp("E|W"))) > 0)
        lon = VGSurveyMonitor::correctValue(strLs.at(idx - 1).toDouble(), strLs.at(idx));

    if (idx > 0 && strLs.size() > idx + 3)
    {
        ret = strLs.at(idx + 1).toInt();
        mnt->SetPosMode(ret);
        mnt->SetSatNum(strLs.at(idx + 2).toInt());
        mnt->SetPrecision(strLs.at(idx + 3).toDouble());
    }

    double alt = 0;
    double altR = 0;
    idx = strLs.indexOf("M");
    if (idx > 0)
        alt = strLs.at(idx - 1).toDouble();
    idx = strLs.indexOf("M", idx + 1);
    if (idx > 0)
        altR = alt + strLs.at(idx - 1).toDouble();

    mnt->SetAltitude(alt);
    mnt->SetRelativeAltitude(altR);

    if (abs(lat) <= 90 && abs(lat) <= 180)
        mnt->SetLatAndLon(lat, lon, true);

    return ret;
}

bool VGBDLocation::_isTimeout() const
{
    return QDateTime::currentMSecsSinceEpoch() - m_tmLstFrame > 300;
}

void VGBDLocation::_send(const QByteArray &arr)
{
    emit _writeBlt(arr);
}

void VGBDLocation::_sendAck(uchar cmd)
{
    QByteArray ret;
    ret.push_back(PACKHEADER);
    switch (cmd)
    {
    case PACKTYPE_INIT:
        if (m_nFramWrite >= 0 || m_arrayRtcm.isEmpty())
            return;
        ret.push_back(6);
        ret.push_back(PACKTYPE_INIT);
        ret.push_back((m_arrayRtcm.size() + PACKFRAM_SIZE - 1) / PACKFRAM_SIZE);
        ret.push_back(char(0));
        break;
    case PACKTYPE_REREQ: //重发数据帧
        ret.push_back(6);
        ret.push_back(PACKTYPE_REREQ);
        ret.push_back(1);
        ret.push_back(m_oth);
        break;
    case PACKTYPE_INITACK: //初始化ACK
        ret.push_back(4);
        ret.push_back(PACKTYPE_INITACK);
        break;
    case PACKTYPE_FINISHFRAM: //完成数据帧
        ret.push_back(4);
        ret.push_back(PACKTYPE_FINISHFRAM);
        break;
    default:
        break;
    }

    if (ret.size() > 2)
        ret.push_back(PACKTAILER);
    else
        ret.clear();

    if (!ret.isEmpty())
        _send(ret);
}

void VGBDLocation::timerEvent(QTimerEvent *event)
{
    if (m_idTimer == event->timerId())
    {
        if (m_bConnect &&QDateTime::currentMSecsSinceEpoch() - m_tmLastGGA > TMOUTLink)
            onLinkConnect(false);
        return;
    }

    QObject::timerEvent(event);
}

void VGBDLocation::onLinkConnect(bool b)
{
    qvgApp->surveyMonitor()->SetConnected(b);
    if (b == m_bConnect)
        return;

    m_bConnect = b;
    if (!b)
    {
        qvgApp->bluetoothManager()->shutdown();
        m_lstFramStat.SetSize(0);
        m_arrayRcv.clear();
        m_arrayRtcm.clear();
    }
}
