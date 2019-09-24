#include "VgNetWork.h"
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QJsonDocument>
VgNetWork::VgNetWork(QObject *parent)
    :QObject(parent)
{
    _pNWreply = NULL;
    _pNetWorkAccMgr = new QNetworkAccessManager(this);
    _pNetWorkRequest = new QNetworkRequest();
}


VgNetWork::~VgNetWork(void)
{
    if(_pNWreply)
        _pNWreply->deleteLater();
}

void VgNetWork::setHeader(QNetworkRequest::KnownHeaders header, const QVariant &value)
{
    _pNetWorkRequest->setHeader(header,value);
}

void VgNetWork::setBody(const QByteArray &array)
{
    _bodyArray = array;
}

void VgNetWork::setUrl(const QByteArray &array)
{
    _urlArray = array;
}

void VgNetWork::setRawHeader(const QByteArray &headerName, const QByteArray &value)
{
    _pNetWorkRequest->setRawHeader(headerName,value);
}

void VgNetWork::netWorkRequest()
{
    if(_pNWreply)
    {
        delete _pNWreply;
        _pNWreply = NULL;
    }
    _pNetWorkRequest->setUrl(QUrl(_urlArray));
    _pNWreply = _pNetWorkAccMgr->post(*_pNetWorkRequest,_bodyArray);
    connect(_pNWreply, SIGNAL(finished()), this, SLOT(networkReplyFinished()));
    connect(_pNWreply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(networkReplyError(QNetworkReply::NetworkError)));
}

void VgNetWork::networkReplyFinished()
{
    if (!_pNWreply)
        return;

    if (_pNWreply->error() != QNetworkReply::NoError)
        return;

    QByteArray reArr = _pNWreply->readAll();
    emit signalNetReply(reArr);

    _pNWreply->deleteLater();
    _pNWreply = NULL;
}

void VgNetWork::networkReplyError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    if (!_pNWreply)
        return;

    switch (error)
    {
    case QNetworkReply::ConnectionRefusedError:
    case QNetworkReply::TimeoutError:
        {
            emit signalNetErr(_pNWreply->url().toString(),"Network anomaly!");
        }
        break;
    default:
        {
            emit signalNetErr(_pNWreply->url().toString(),_pNWreply->errorString());
        }
        break;
    }

    _pNWreply->deleteLater();
    _pNWreply = NULL;
}

