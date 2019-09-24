#include "VGLogDataObject.h"
#include <QDebug>
#include <QQmlEngine>
#include <QFile>

#include <QElapsedTimer>
#include <memory>
#include <QBitArray>
#include <QtMath>

static QLocale kLocale;


VGLogEntry::VGLogEntry()
{

}

VGLogEntry::VGLogEntry(uint logId, const QDateTime& dateTime, uint logSize, bool received)
    : _logID(logId)
    , _logSize(logSize)
    , _logTimeUTC(dateTime)
    , _received(received)
    , _selected(false)
{
    _statusStr = "Pending";
}


//-----------------------------------------------------------------------------
VGLogModel::VGLogModel(QObject* parent)
    : QAbstractListModel(parent)
{

}

VGLogEntry* VGLogModel::get(int index)const
{
    if (index < 0 || index >= _logEntries.count()) {
        return NULL;
    }
    return _logEntries[index];
}

//-----------------------------------------------------------------------------
int VGLogModel::count() const
{
    return _logEntries.count();
}

//-----------------------------------------------------------------------------
void VGLogModel::append(VGLogEntry* object)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
    _logEntries.append(object);
    endInsertRows();
    emit countChanged();
}

//-----------------------------------------------------------------------------
void VGLogModel::clear(void)
{
    if(!_logEntries.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, _logEntries.count());
        while (_logEntries.count()) {
            VGLogEntry* entry = _logEntries.last();
            if(entry) entry->deleteLater();
            _logEntries.removeLast();
        }
        endRemoveRows();
        emit countChanged();
    }
}

//-----------------------------------------------------------------------------
VGLogEntry* VGLogModel::operator[](int index)
{
    return get(index);
}

//-----------------------------------------------------------------------------
int VGLogModel::rowCount(const QModelIndex& /*parent*/) const
{
    return _logEntries.count();
}

//-----------------------------------------------------------------------------
QVariant VGLogModel::data(const QModelIndex & index, int role) const {
    if (index.row() < 0 || index.row() >= _logEntries.count())
        return QVariant();
    if (role == ObjectRole)
        return QVariant::fromValue(_logEntries[index.row()]);
    return QVariant();
}

//-----------------------------------------------------------------------------
QHash<int, QByteArray> VGLogModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[ObjectRole] = "logEntry";
    return roles;
}

//----------------------------------------------------------------------------------------
QString VGLogEntry::sizeStr() const
{
    return kLocale.toString(_logSize);
}

VGLogDataObject::VGLogDataObject(QObject *parent) : QObject(parent)
, m_logModel(new VGLogModel(this))
{
    m_downStatus = 0;
}

QString VGLogDataObject::fileName()const
{
    return m_fileName;
}

void VGLogDataObject::setFileName(const QString &name)
{
    m_fileName = name;
}

double VGLogDataObject::fileSize()const
{
    return m_fileSize;
}

void VGLogDataObject::setFileSize(int size)
{
    m_fileSize = size;
}

int VGLogDataObject::downStatus()const
{
    return m_downStatus;
}

void VGLogDataObject::setDownStatus(int status)
{
    m_downStatus = status;
    emit downStatusChanged(status);
}

void VGLogDataObject::refresh()
{
    qDebug() << "refresh ......" ;
    emit sigUpdateLogList();
}

void VGLogDataObject::sltReceiveLogEntry(long time_utc, long size, int id, int num_logs, bool downloaded)
{
    //qDebug() << "receive log Entry ......" ;
	if (!m_logModel)
		return;

    if(downloaded)
    {
        VGLogEntry *entry = new VGLogEntry(id);
        entry->setSize(size);
        entry->setTime(QDateTime::fromTime_t(time_utc));
        entry->setStatusStr(QString::fromLocal8Bit("下载完成"));
        entry->setStatus(2);
        entry->setReceived(true);
        m_logModel->append(entry);
    }
    else
    {
        //遍历已经存在的
        bool bFind = false;
        for(int i = 0; i < m_logModel->count(); i++)
        {
            VGLogEntry *entry = (*m_logModel)[i];
            if(entry->received())
            {//downloaded 飞控重启后，每次日志的id并不一定和上次的id完全相同，所以这里这判断id；
                if(/*entry->id() == id && */entry->size() == size && entry->time().toTime_t() == (uint)time_utc)
                {
                    bFind = true;
                    break;
                }
            }
            else
            {
                if(entry->id() == id)
                {//当前正在记录的日志
                    entry->setSize(size);
                    entry->setTime(QDateTime::fromTime_t(time_utc));
                    entry->setStatusStr(QString::fromLocal8Bit("未下载"));
                    entry->setStatus(0);
                    bFind = true;
                    break;
                }
            }
        }

        if(!bFind)
        {
            VGLogEntry *entry = new VGLogEntry(id);
            entry->setSize(size);
            entry->setTime(QDateTime::fromTime_t(time_utc));
            entry->setStatusStr(QString::fromLocal8Bit("未下载"));
            entry->setStatus(0);
            entry->setReceived(false);
            m_logModel->append(entry);
        }
    }
}

void VGLogDataObject::downLoadLog(int index, int logId)
{
	if (!m_logModel)
		return;

    for(int i = 0; i < m_logModel->count(); i++)
    {
        VGLogEntry* logEntry = (*m_logModel)[i];
        if(i == index && logId == logEntry->id() && logEntry->status() == 1)
        {
            emit sigRequestLogData(logEntry->id(), logEntry->size(), logEntry->time().toTime_t());
            setDownStatus(1);
        }
    }
}

void VGLogDataObject::sltUpdateLogDownloadStatus(int logId, ulong time_utc, const QString &status)
{
	if (!m_logModel)
		return;

    for(int i = 0; i < m_logModel->count(); i++)
    {
        VGLogEntry* logEntry = (*m_logModel)[i];
        if(logEntry->id() == logId && logEntry->time().toTime_t() == (uint)time_utc)
        {
            logEntry->setStatusStr(status);
            if(status == QString::fromLocal8Bit("下载完成"))
            {
                logEntry->setStatus(2);
                logEntry->setReceived(true);
                qDebug() << "download " << logId << status;
            }
            else if(status == QString::fromLocal8Bit("Error"))
            {
                logEntry->setStatus(0);                
                qDebug() << "download " << logId << status;
            }
            else
            {
                logEntry->setStatus(1);
            }
            break;
        }
    }
}

void VGLogDataObject::clearLogModel()
{
    m_logModel->clear();
}
