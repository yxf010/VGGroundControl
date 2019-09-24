#ifndef VGTCPCLIENT_H
#define VGTCPCLIENT_H

#include <QObject>

class QTcpSocket;
class VGTcpClient : public QObject
{
    Q_OBJECT
public:
    explicit VGTcpClient(QObject *parent = 0);
    ~VGTcpClient();

    void InitSock();
    void connetServer(const QString &hostName, int port, int timeout = 3000);
    void DisconnectTcp();
public slots:
    void sltSendRequestInfo(const QString &name, const QByteArray &array);
private slots:
    //线程操作返回结果
    void OnTcpConnected();
    void OnReadReady();
    void OnDisconnected();
    void init(); //初始化socket对象，绑定相关信号
    void closeTcp();
signals:
    void socketInit();
    void disconnectTcp();
    void sigInfoAck(const QString &name, const QByteArray &);           //收到应答消息
    void sigCommunicateResult(bool);
private:
    QTcpSocket      *m_tcpSocket;
    QByteArray      m_arrayRead;
};

#endif // VGTCPCLIENT_H
