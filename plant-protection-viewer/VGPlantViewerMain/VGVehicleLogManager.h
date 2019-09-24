#ifndef VGVEHICLELOGMANAGER_H
#define VGVEHICLELOGMANAGER_H

#include <QObject>
#include <QFile>
#include <QElapsedTimer>
#include <QBitArray>
#include <QtMath>
#include <QTimer>
#include <QDateTime>

#include "LinkInterface.h"
#include "VGVehicle.h"

#define kTimeOutMilliseconds 500
#define kGUIRateMilliseconds 17
#define kTableBins           512
#define kChunkSize           (kTableBins * MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN)

struct LogDownloadData {
    LogDownloadData(uint vehicleId, uint type, uint logId, ulong logSize, ulong time_utc);
    QBitArray     chunk_table;
    ulong         current_chunk;
    QFile         file;
    QString       filename;
    uint          _vehicleID;
    uint          _vehicleType;
    ulong         _logTime;
    uint          _logID;        //日志文件id
    ulong         _logSize;      //日志总大小
    //QGCLogEntry*  entry;
    ulong          written;      //已经下载数据包数
    QElapsedTimer elapsed;
    bool          _downloading;   //是否正在下载

    void advanceChunk()
    {
           current_chunk++;
           chunk_table = QBitArray(chunkBins(), false);
    }

    // The number of MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN bins in the current chunk
    uint32_t chunkBins() const
    {
        return qMin(qCeil((_logSize - current_chunk*kChunkSize)/static_cast<qreal>(MAVLINK_MSG_LOG_DATA_FIELD_DATA_LEN)),
                    kTableBins);
    }

    // The number of kChunkSize chunks in the file
    uint32_t numChunks() const
    {
        return qCeil(_logSize / static_cast<qreal>(kChunkSize));
    }

    // True if all bins in the chunk have been set to val
    bool chunkEquals(const bool val) const
    {
        return chunk_table == QBitArray(chunk_table.size(), val);
    }

    //日志是否下载完
    bool isLogComplete()
    {
        return chunkEquals(true) && (current_chunk + 1) == numChunks();
    }

};


class VGVehicleLogManager : public QObject
{
    Q_OBJECT
public:
    explicit VGVehicleLogManager(QObject *parent = 0);
    ~VGVehicleLogManager();

private:
    QList<LogDownloadData*>  m_lstLogDownloadData;
    QString                 m_strLogDir;
    VGVehicle*              m_vehicle;
    QTimer*                 m_timer;    //下载队列定时器

private:
    bool    _prepareLogDownload(LogDownloadData* downloadData);
    void    _requestLogData    (int vehicleId, uint id, ulong offset = 0, ulong count = 0xFFFFFFFF);
    void    _findMissingData   (LogDownloadData* downloadData);
    void    _requestLogList(int vehicleId, int start, int end);

signals:
    void    sigRequsetLogList(int vehicleid, int start, int end);
    void    sigRequestLogData(int vehicleId, uint id, ulong offset, ulong count);
    void    sigUpdateDownloadStatus(int logId, uint32_t time_utc, QString status);
    void    sigUpdateDownloadedLog(uint32_t time_utc, uint32_t size, uint16_t id, uint16_t num_logs, bool downloaded);
    void    sigAllDownloaded();//所有文件下载完成
    void    sigSetDownloading(bool downloading);

public slots:
    void    sltRequestLogList();
    void    sltRequestLogData(int logId, long size, long time_utc);
    void    sltReceiveLogData(uint32_t ofs, uint16_t id, uint8_t count, const uint8_t* data);

    void    setVehicle(VGVehicle* vehicle);
    void    setLogDownloadDir(const QString &strDir);
    void    loadDownloadedLog();                //加载已经下载完成的日志列表

private slots:
    void    sltProcessDownload();

};

#endif // VGVEHICLELOGMANAGER_H
