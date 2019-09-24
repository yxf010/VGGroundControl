#include "VGTcpSocket.h"

VGTcpSocket::VGTcpSocket(QObject *parent):QTcpSocket(parent)
{
    m_timer.setInterval(500);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(sltTimeout()));
}

void VGTcpSocket::sendData(const QByteArray &data)
{
    if(isValid())
    {
        write(data, data.size());
        if(!waitForBytesWritten())
        {
            qDebug() << "write timeout";
            sendData(data);
        }
    }
}

void VGTcpSocket::disConnectTcp()
{
    this->disconnectFromHost();
    if (this->state() != QAbstractSocket::UnconnectedState)
        this->waitForDisconnected();
}

bool VGTcpSocket::connectServer(const QString &hostName, int port, int timeout)
{
    connectToHost(hostName, port);
    if(waitForConnected(timeout))
    {
        qDebug()<<"conncet to server is timeout";
        emit sigResult(-1);
        return false;
    }

    emit sigResult(0);
    return true;
}

void VGTcpSocket::sltTimeout()
{
    if(isValid())
        emit readyRead();
}
