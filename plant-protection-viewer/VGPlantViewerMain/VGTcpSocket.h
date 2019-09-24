#ifndef VGTCPSOCKET_H
#define VGTCPSOCKET_H

#include <QTcpSocket>
#include <QTimer>

class VGTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit VGTcpSocket(QObject *parent = 0);

    void sendData(const QByteArray &data);//发送信号的槽
    void disConnectTcp();
    bool connectServer(const QString &hostName, int port, int timeout);
private slots:
    void sltTimeout();
signals:
    void readData(const int, const QString &,const quint16,const QByteArray &);//发送获得用户发过来的数据
    void sockDisConnect(const int ,const QString &,const quint16 );//断开连接的用户信息

    //操作结果 -1 连接服务器失败 0 连接服务器成功；
    void sigResult(int index);
private:
    QTimer m_timer;
};

#endif // VGTCPSOCKET_H
