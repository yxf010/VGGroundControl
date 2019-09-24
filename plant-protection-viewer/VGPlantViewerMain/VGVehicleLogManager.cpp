#include "VGVehicleLogManager.h"
#include <QDebug>
#include <QLocale>
#include <QSettings>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include "VGApplication.h"

static QLocale g_kLocale;

//----------------------------------------------------------------------------------------
LogDownloadData::LogDownloadData(uint vehicleId, uint type, uint logId, ulong logSize, ulong time_utc)
    : _vehicleID(vehicleId)
    , _logID(logId)
    , written(0)
    , _logSize(logSize)
    , _vehicleType(type)
    , _logTime(time_utc)
    , _downloading(false)
{

}

VGVehicleLogManager::VGVehicleLogManager(QObject *parent)
    : QObject(parent)
    , m_vehicle(NULL)
{
    m_strLogDir = "";
    m_timer = NULL;
}

VGVehicleLogManager::~VGVehicleLogManager()
{

}

void VGVehicleLogManager::setVehicle(VGVehicle *vehicle)
{
    m_vehicle = vehicle;
    if(m_timer)
    {
        if(m_timer->isActive())
        {
            m_timer->stop();
        }
        delete m_timer;
        m_timer = NULL;
    }

    m_timer = new QTimer();
    connect(m_timer, &QTimer::timeout, this, &VGVehicleLogManager::sltProcessDownload);
    m_timer->setInterval(500);
}

void VGVehicleLogManager::sltRequestLogData(int logId, long size, long time_utc)
{
    if(m_vehicle == NULL)
    {
        qDebug() << "no active vehicle";
        return;
    }

    for(int i = 0; i < m_lstLogDownloadData.size(); i++)
    {
        LogDownloadData* logData = m_lstLogDownloadData.at(i);
        if(logData)
        {
            if(logData->_vehicleID == m_vehicle->id() && logData->_logID == logId)
            {
                return;
            }
        }
    }

    LogDownloadData* newLogDownload = new LogDownloadData(m_vehicle->id(), m_vehicle->firewareType(), logId, size, time_utc);

    m_lstLogDownloadData.append(newLogDownload);
    m_timer->start();
}

void VGVehicleLogManager::sltProcessDownload()
{
    for(int i = 0; i < m_lstLogDownloadData.size(); i++)
    {
        LogDownloadData* logData = m_lstLogDownloadData.at(i);
        if(logData)
        {
            if(!logData->_downloading)
            {
                if(_prepareLogDownload(logData))
                {
                    logData->_downloading = true;
                    _requestLogData(logData->_vehicleID, logData->_logID, 0, logData->chunk_table.size()*MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN);
                    m_timer->stop();
                    break;
                }                
            }
            else if(logData->_downloading)
            {
                _findMissingData(logData);
            }
        }
    }

    if(m_lstLogDownloadData.size() <= 0)
    {
        m_timer->stop();
    }
}

bool VGVehicleLogManager::_prepareLogDownload(LogDownloadData *downloadData)
{
    bool result = false;
    QString ftime;
    if((QDateTime::fromTime_t(downloadData->_logTime)).date().year() < 2010)
    {
        ftime = "UnknownDate";
    }
    else
    {
        ftime = (QDateTime::fromTime_t(downloadData->_logTime)).toString("yyyy-M-d-hh-mm-ss");
    }

    QString strFileName = QString("log_") + QString::number(downloadData->_vehicleID) + "_" + QString::number(downloadData->_logID) + "_" + ftime;
    if(downloadData->_vehicleType == MAV_AUTOPILOT_PX4)
    {
        strFileName += ".px4log";
    }
    else
    {
        strFileName += ".bin";
    }
    downloadData->filename = strFileName;
    downloadData->file.setFileName(m_strLogDir + strFileName);

    if (downloadData->file.exists())
    {//如果文件存在，自动修改文件名为_1
        uint num_dups = 0;
        QStringList filename_spl = downloadData->filename.split('.');
        //qDebug() << "file is exist:" << filename_spl;
        do {
            num_dups += 1;
            downloadData->file.setFileName(m_strLogDir + filename_spl[0] + '_' + QString::number(num_dups) + '.' + filename_spl[1]);
        } while( downloadData->file.exists());
    }

    //-- Create file
    if (!downloadData->file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to create log file:" <<  downloadData->filename;
    }
    else
    {
        //-- Preallocate file
        if(!downloadData->file.resize(downloadData->_logSize))
        {
            qWarning() << "Failed to allocate space for log file:" <<  downloadData->filename;
        }
        else
        {
            downloadData->current_chunk = 0;
            downloadData->chunk_table = QBitArray(downloadData->chunkBins(), false);
            downloadData->elapsed.start();
            result = true;
        }
    }
    if(!result)
    {
        if (downloadData->file.exists())
        {
            downloadData->file.remove();
        }
        //_downloadData->entry->setStatus(QString("Error"));
        delete downloadData;
        downloadData = NULL;
    }
    return result;
}

void VGVehicleLogManager::_requestLogData(int vehicleId, uint id, ulong offset, ulong count)
{
    emit sigRequestLogData(vehicleId, id, offset, count);
}

void VGVehicleLogManager::sltReceiveLogData(uint32_t ofs, uint16_t id, uint8_t count, const uint8_t *data)
{
    //qDebug() << "ofs:" << ofs << "id:" << id << "count:" << count;
    LogDownloadData* logData = NULL;
    bool bFindLogFile = false;
    int index = -1;
    for(int i = 0; i < m_lstLogDownloadData.size(); i++)
    {
        logData = m_lstLogDownloadData.at(i);
        if(logData->_logID == id)
        {
            index = i;
            bFindLogFile = true;
            break;
        }
    }

    if(!bFindLogFile)
    {
        qWarning() << "Received log data for wrong log" << id ;
        return;
    }

    if ((ofs % MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN) != 0)
    {
        qWarning() << "Ignored misaligned incoming packet @" << ofs;
        return;
    }


    bool result = false;
    uint32_t timeout_time = kTimeOutMilliseconds;
    if(ofs <= logData->_logSize)
    {
        const uint32_t chunk = ofs / kChunkSize;
        if (chunk != logData->current_chunk)
        {
            qWarning() << "Ignored packet for out of order chunk" << chunk;
            return;
        }

        const uint16_t bin = (ofs - chunk*kChunkSize) / MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN;

        if(logData->chunk_table.testBit(bin))
        {//已经下载过的bin
            return;
        }

        if (bin >= logData->chunk_table.size())
        {
            qWarning() << "Out of range bin received";
        }
        else
        {
            logData->chunk_table.setBit(bin);
        }

        if (logData->file.pos() != ofs)
        {
            // Seek to correct position
            if (!logData->file.seek(ofs))
            {
                qWarning() << "Error while seeking log file offset";
                return;
            }
        }

        //-- Write chunk to file
        if(logData->file.write((const char*)data, count))
        {
            logData->written += count;
            if (logData->elapsed.elapsed() >= kGUIRateMilliseconds)
            {
                //-- Update status
                QString comma_value = g_kLocale.toString(uint(logData->written));
                emit sigUpdateDownloadStatus(logData->_logID, (uint32_t)logData->_logTime, QString(comma_value));
                logData->elapsed.start();
            }
            result = true;

//            //-- Reset timer
            m_timer->start(timeout_time);

            //-- Do we have it all?
            if(logData->isLogComplete())
            {
                if (QSettings *settings = qvgApp->GetSettings())
                {
                    settings->beginGroup("log");
                    QString strDownloadLog = settings->value("downloaded", QString()).toString();
                    if (!strDownloadLog.isEmpty())
                    {
                        strDownloadLog += ",";
                    }
                    strDownloadLog += logData->filename;
                    settings->setValue("log/downloaded", QVariant(strDownloadLog));
                    settings->endGroup();
                    emit sigUpdateDownloadStatus(logData->_logID, (uint32_t)logData->_logTime, QString::fromLocal8Bit("下载完成"));
                    logData->_downloading = false;
                }

                m_lstLogDownloadData.removeAt(index);
                delete logData;
                logData = NULL;


                if(m_lstLogDownloadData.count() == 0)
                {
                    emit sigAllDownloaded();
                    if(m_vehicle)
                    {
                        m_vehicle->setConnectionLostEnabled(true);
                    }
                }

                m_timer->start(timeout_time);
            }
            else if (logData->chunkEquals(true))
            {
                logData->advanceChunk();
                _requestLogData(logData->_vehicleID, logData->_logID,
                                logData->current_chunk*kChunkSize,
                                logData->chunk_table.size()*MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN);
            }
        }
        else
        {
            qWarning() << "Error while writing log file chunk";
            logData->_downloading = false;
        }
    }
    else
    {
        qWarning() << "Received log offset greater than expected";
        logData->_downloading = false;
    }

    if(!result)
    {
        qDebug() << "emit status to UI";
        emit sigUpdateDownloadStatus(logData->_logID, (uint32_t)logData->_logTime, QString("Error"));
        logData->_downloading = false;
    }

}

void VGVehicleLogManager::setLogDownloadDir(const QString &strDir)
{
    if(strDir.isEmpty() || strDir.length() < 3)
    {
        return;
    }

	m_strLogDir = strDir;
	if (!strDir.endsWith("/") && !strDir.endsWith("\\"))
		m_strLogDir += "/";
}

void VGVehicleLogManager::loadDownloadedLog()
{
    QStringList lstDownloadedLog;
    if (QSettings *settings = qvgApp->GetSettings())
    {
        settings->beginGroup("log");
        QString strDownloadLog = settings->value("downloaded", QVariant("")).toString();
        settings->endGroup();
        lstDownloadedLog = strDownloadLog.split(",");
    }

    if(lstDownloadedLog.size() <= 0)
        return;
 
    qDebug() << "loadDownloadedLog......";
    QString strDir = m_strLogDir;

    for(int i = 0; i < lstDownloadedLog.size(); i++)
    {
        QString strLogName = lstDownloadedLog.at(i);
        QString logId = strLogName.section("_", 2, 2);
        QString vehicleId = strLogName.section("_", 1, 1);
        QString strTime = strLogName.section("_", 3, 3).section(".", 0, 0);
        QDateTime dateTime = QDateTime::fromString(strTime, "yyyy-M-d-hh-mm-ss");
        long utc_time = dateTime.toTime_t();

        QFileInfo file(strDir + strLogName);
        long size = file.size();

        //qDebug() << "downloaded......" << logId << size << strTime;
        emit sigUpdateDownloadedLog(utc_time, size, logId.toInt(), lstDownloadedLog.count(), true);
    }
}

void VGVehicleLogManager::_findMissingData(LogDownloadData *downloadData)
{

    if (downloadData->isLogComplete()) {
         //_receivedAllData();
         return;
    } else if (downloadData->chunkEquals(true)) {
        downloadData->advanceChunk();
    }

    uint16_t start = 0, end = 0;
    const int size = downloadData->chunk_table.size();
    for (; start < size; start++) {
        if (!downloadData->chunk_table.testBit(start)) {
            break;
        }
    }

    for (end = start; end < size; end++) {
        if (downloadData->chunk_table.testBit(end)) {
            break;
        }
    }

    const uint32_t pos = downloadData->current_chunk*kChunkSize + start*MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN,
                   len = (end - start)*MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN;
    _requestLogData(downloadData->_vehicleID, downloadData->_logID, pos, len);
}

void VGVehicleLogManager::sltRequestLogList()
{
    if(m_vehicle)
    {
        _requestLogList(m_vehicle->id(), 0, 49);
    }
}

void VGVehicleLogManager::_requestLogList(int vehicleId, int start, int end)
{
    emit sigRequsetLogList(vehicleId, start, end);
}
