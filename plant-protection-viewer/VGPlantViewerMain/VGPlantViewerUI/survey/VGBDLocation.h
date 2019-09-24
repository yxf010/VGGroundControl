#ifndef __VGBDLACATION_H__
#define __VGBDLACATION_H__

#include <QObject>
#include <QString>
#include <QByteArray>

class LinkInterface;
class VGBDLocation : public QObject
{
    Q_OBJECT
private:
    class BitList
    {
    public:
        BitList(uchar count);
        void SetSize(uchar count);
        int GetSize()const;
        void SetValue(uchar index, bool b = true);
        bool GetValue(uchar index)const;
        int GetFirstFrom(bool b, uchar from = 0)const;
        int GetFirstTo(bool b, uchar to = 0)const;
    private:
        int         m_count;
        uchar       m_lstFrame[8];
    };
public:
    VGBDLocation(QObject *p=NULL);
    void Linked(LinkInterface *link);

    void SendRTCM(const QList<uint16_t> idxs, const QList<QByteArray> &rtcms);
protected:
    uchar _checkFrame(const QByteArray &arr);
    bool  _checkPack();
    uchar _prcsPackInit(const QByteArray &arr);
    void  _parsePack(const QByteArray &arr);
    void _parseNmea(const QString &str);
    void _powPer(uchar per);
    int _parPos(const QStringList &strLs);
    bool _isTimeout()const;
    void _send(const QByteArray &arr);
    void _sendAck(uchar cmd);

    void timerEvent(QTimerEvent *event);
private slots:
    void onWtiteRTCM(const QByteArray &data);
    void onReceive(LinkInterface *link, const QByteArray &data);
signals:
    void _writeBlt(const QByteArray &array);
    void _rtcmWrite(const QByteArray &arr);
private:
    bool _isIdle()const;
    void _begWriteRtcm();
    bool _writeRtcm(uchar frame);
    void onLinkConnect(bool b);
private:
    QByteArray      m_arrayRcv;
    QByteArray      m_arrayRtcm;
    QByteArray      m_content;
    uchar           m_type;
    uchar           m_oth;
    ushort          m_lenth;
    BitList         m_lstFramStat;
    unsigned        m_tmLstFrame;
    int             m_nFramWrite;
    bool            m_bIdle;
    bool            m_bConnect;
    qint64          m_tmLastRctm;
    qint64          m_tmLastGGA;
    int             m_idTimer;
};

#endif // __VGBDLACATION_H__
