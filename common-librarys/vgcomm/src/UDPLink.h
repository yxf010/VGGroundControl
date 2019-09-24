#ifndef UDPLINK_H
#define UDPLINK_H

#include <QString>

#if defined(QGC_ZEROCONF_ENABLED)
#include <dns_sd.h>
#endif

//#include "QGCConfig.h"
#include "LinkCommand.h"
#include "LinkInterface.h"
#include "vgcomm_global.h"

class  QUdpSocket;
class  QThread;
class VGCOMMSHARED_EXPORT UDPCommand : public LinkCommand
{
    Q_OBJECT

    Q_PROPERTY(QString   host   READ GetHost  NOTIFY hostChanged)
    Q_PROPERTY(quint16   localPort   READ localPort  NOTIFY localPortChanged)
public:
    UDPCommand(QObject *p=NULL);
    ~UDPCommand();

    void SetLastRcvHost(const QString& host, int port);
    bool GetLastRcvHost(QString& host, int& port);
    quint16 localPort()const;
    void setLocalPort   (quint16 port);
    QString GetHost()const;
    bool IsValid()const;

    Q_INVOKABLE void setHost(const QString host, uint16_t port);

    void        loadSettings    (QSettings& settings, const QString& root);
    void        saveSettings    (QSettings& settings, const QString& root);
    void        updateSettings();
    LinkType    type()const;
    void        connectLink();
    QString getName()const;
    void setName(const QString &s);
signals:
    void localPortChanged(quint16 port);
    void hostChanged(const QString &host);
private:
    void _updateHostList    ();
private:
    QMutex              _confMutex;
    QString             m_hostName;         ///< Exposed to QML
    quint16             m_localPort;
    QString             m_rcvHost;
    quint16             m_rcvPort;         ///< ("host", port)
};

class VGCOMMSHARED_EXPORT UDPLink : public LinkInterface
{
    Q_OBJECT
public:
    void requestReset() { }
    bool isConnected() const;

    // Extensive statistics for scientific purposes
    qint64 getConnectionSpeed() const;
    qint64 getCurrentInDataRate() const;
    qint64 getCurrentOutDataRate() const;
public slots:
    void writeBytes(const QByteArray &array);
protected slots:
    void readBytes();
    void sendUdp();
private:
    // Links are only created/destroyed by LinkManager so constructor/destructor is not public
    UDPLink(UDPCommand* config);
    ~UDPLink();

    virtual bool _connect(void);
    virtual void _disconnect(void);

    bool _hardwareConnect();
    void _restartConnection();

    void _registerZeroconf(uint16_t port, const std::string& regType);
    void _deregisterZeroconf();
    bool _dequeBytes();
    UDPCommand * udpCommand()const;
signals:
    void _sendUdp();
private:
    friend class UDPCommand;

    QUdpSocket          *m_socket;
    QThread             *m_thread;
    bool                m_connectState;
    QList<QByteArray>   m_outQueue;
#if defined(QGC_ZEROCONF_ENABLED)
    DNSServiceRef  _dnssServiceRef;
#endif

};

#endif // UDPLINK_H
