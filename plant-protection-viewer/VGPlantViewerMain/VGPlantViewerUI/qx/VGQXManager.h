#ifndef __VGQXMANAGER_H__
#define __VGQXMANAGER_H__

#include <QObject>
#include <QByteArray>

class QTcpSocket;
class QThread;
class QGeoCoordinate;

class VGQXManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString url READ GetUrl WRITE SetUrl NOTIFY urlChanged)
    Q_PROPERTY(quint16 port READ GetPort WRITE SetPort NOTIFY portChanged)
    Q_PROPERTY(QString acc READ GetAccount WRITE SetAccount NOTIFY accChanged)
    Q_PROPERTY(QString pswd READ GetPassword WRITE SetPassword NOTIFY pswdChanged)
    Q_PROPERTY(bool started READ GetStart WRITE  SetStart NOTIFY startedChanged)
    Q_PROPERTY(bool rtcm READ GetRtcm NOTIFY rtcmChanged)
public:
    enum {
        FRAME_HEADER = 0xd3,

        TYPE_STATION = 1005,
        TYPE_GPS = 1074,
        TYPE_HALILEO = 1094,
        TYPE_QZSS = 1114,
        TYPE_BEIDOU = 1124,
    };
public:
    explicit VGQXManager(QObject *parent = NULL);
    ~VGQXManager();

    void SendGGA(const QByteArray &gga);
    bool GetRtcm()const;
public:
    static QByteArray GenerateGGa(const QGeoCoordinate &coor, double altRel=0, int sat = 10, double pr = 99);
protected:
    void SetUrl(const QString &url);
    QString GetUrl()const;
    void SetPort(quint16 p);
    quint16 GetPort()const;
    void SetAccount(const QString &acc);
    QString GetAccount()const;
    void SetPassword(const QString &pswd);
    QString GetPassword()const;
    void SetStart(bool b);
    bool GetStart()const;

    Q_INVOKABLE void saveConfig();
    void loadConfig();
    void SetRtcm(bool b);

    void timerEvent(QTimerEvent *event);
protected slots:
    void onStartChanged(bool b);
    void onReadyRead();
    void onConneted();
    void onDisconnected();

    void onWriteGGA(const QByteArray &gga);
    void onServerReady();
signals:
    void rtcmRead(const QList<uint16_t> idxs, const QList<QByteArray> &rtcms);
private:
    void _writeGGa(const QByteArray &gga);
    void _beginGet(const QString &url);
    void _addEle(const QString &key, const QByteArray &value);
    void _addAuthorization(const QString &acc, const QString &pswd);
    void _endGet();
    int _parseRtcm(const QByteArray &value);
signals:
    void urlChanged(const QString &url);
    void portChanged(quint16 port);
    void accChanged(const QString &acc);
    void pswdChanged(const QString &pswd);
    void startedChanged(bool b);
    void writeGGA(const QByteArray &gga);
    void rtcmChanged(bool);
private:
    QString     m_strUrl;
    QString     m_strAcc;
    QString     m_strPswd;
    bool        m_bStart;
    bool        m_bAcceptGGA;
    quint16     m_port;
    qint64      m_tmLastGGA;
    qint64      m_tmLastRTCM;

    QTcpSocket  *m_socket;
    QThread     *m_thread;
    QByteArray  m_arraySend;
    QByteArray  m_arrayRecv;
    QByteArray  m_arrayGGa;
    int         m_idTimer;
    bool        m_bRtcm;
};

#endif // __VGQXMANAGER_H__
