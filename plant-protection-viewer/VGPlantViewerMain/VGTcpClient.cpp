#include "VGTcpClient.h"
#include "JQChecksum.h"
#include <QDebug>
#include <QtEndian>
#include <QTcpSocket>

#include "VGGlobalFun.h"

#define PROTOFLAG "das.proto."

VGTcpClient::VGTcpClient(QObject *parent) : QObject(parent)
, m_tcpSocket(NULL)
{
    connect(this, &VGTcpClient::socketInit, this, &VGTcpClient::init);
    connect(this, &VGTcpClient::disconnectTcp, this, &VGTcpClient::closeTcp);
}

VGTcpClient::~VGTcpClient()
{
}

void VGTcpClient::InitSock()
{
    emit socketInit();
}

void VGTcpClient::init()
{
    if(!m_tcpSocket)
    {
        if (m_tcpSocket = new QTcpSocket())
        {
            m_tcpSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
            connect(m_tcpSocket, &QTcpSocket::readyRead, this, &VGTcpClient::OnReadReady);
            connect(m_tcpSocket, &QTcpSocket::disconnected, this, &VGTcpClient::OnDisconnected);
            connect(m_tcpSocket, &QTcpSocket::connected, this, &VGTcpClient::OnTcpConnected);
        }
    }
}

void VGTcpClient::connetServer(const QString &hostName, int port, int timeout)
{
    if (hostName.isEmpty() || port == 0)
        return ;

    if (m_tcpSocket)
    {
        m_tcpSocket->connectToHost(hostName, port);
        if (!m_tcpSocket->waitForConnected(timeout))
            emit sigCommunicateResult(false);
    }
}

void VGTcpClient::DisconnectTcp()
{
    emit disconnectTcp();
}

void VGTcpClient::closeTcp()
{
    qDebug() << "client is closed;";
    if(m_tcpSocket)
        m_tcpSocket->close();
}

void VGTcpClient::OnReadReady()
{
    QTcpSocket *tcpSocket = qobject_cast<QTcpSocket*>(sender());
    if (!tcpSocket)
        return;

    if(tcpSocket->bytesAvailable() <= 0)
        return;

    m_arrayRead += tcpSocket->readAll();
    while(m_arrayRead.size()>18)
    {
        int proto = m_arrayRead.indexOf(PROTOFLAG, 8);
        if (proto < 0)
        {
            if (m_arrayRead.length() > 17)//长度>=18才有Protobuff标准
                m_arrayRead.right(17);
            break;
        }

        if (proto>8)
            m_arrayRead = m_arrayRead.mid(proto - 8);
        proto = 8;
        int length = VGGlobalFunc::bytesToInt(m_arrayRead);
        if (length < 18 || length>1024*1024)//数据包<1M >18字节
        {
            m_arrayRead = m_arrayRead.mid(proto + 10);
            continue;
        }
        if (length+4 > m_arrayRead.size())
            break;

        uint32_t crc = VGGlobalFunc::bytesToInt(m_arrayRead.mid(length, 4));
        uint32_t crcData = JQChecksum::crc32(m_arrayRead.mid(4, length-4));
        if (crc != crcData)
        {
            m_arrayRead = m_arrayRead.mid(18);
            continue;
        }
        int nameLen = VGGlobalFunc::bytesToInt(m_arrayRead.mid(4,4));
        QString name = m_arrayRead.mid(8, nameLen);

        int protobufLen = length - nameLen - 8;
        emit sigInfoAck(name, m_arrayRead.mid(8+nameLen, protobufLen));
        m_arrayRead = m_arrayRead.mid(length + 4);
    }
}

void VGTcpClient::OnDisconnected()
{
    emit sigCommunicateResult(false);
}

void VGTcpClient::OnTcpConnected()
{
    emit sigCommunicateResult(true);
}

void VGTcpClient::sltSendRequestInfo(const QString &name, const QByteArray &array)
{
    if(m_tcpSocket)
    {
        QByteArray pData;
        pData.clear();

        int nNameLen = name.length() + sizeof(char);
        int nTotal = sizeof(int) * 2 + nNameLen + array.size();

        QByteArray pTotal = VGGlobalFunc::intToBytes(nTotal);
        pData.append((const char*)pTotal, sizeof(int));
        pData.append(VGGlobalFunc::intToBytes(nNameLen));
        pData.append(name.toLatin1(), nNameLen);
        pData.append(array.data(), array.size());

        QByteArray pCrc32Data = pData.mid(sizeof(int));
        unsigned int crc32 = JQChecksum::crc32(pCrc32Data);
        pData += VGGlobalFunc::intToBytes(crc32);

        m_tcpSocket->write(pData);
    }
    else
    {
        qDebug() << "can not connect to server";
    }
}
