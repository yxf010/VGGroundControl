#ifndef VGGETGPSLOCATION_H
#define VGGETGPSLOCATION_H

#include <QObject>
#include <QGeoPositionInfoSource>
#include <QGeoSatelliteInfoSource>
#include <QTimer>


class VGGetGpsLocation : public QObject
{
    Q_OBJECT
    //gps错误信息
    Q_PROPERTY(int errorInfo READ getErrorInfo WRITE setErrorInfo NOTIFY errorInfoChanged)
    Q_PROPERTY(QString result READ getResult WRITE setResult NOTIFY resultChanged)
    //gps数据更新频率
    Q_PROPERTY(int updateInterval READ updateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)
public:
    explicit VGGetGpsLocation(QObject *parent = 0);
    ~VGGetGpsLocation();

    int     updateInterval() { return m_updateInterval; }
    void    setUpdateInterval(int interval) { m_updateInterval = interval; emit updateIntervalChanged(interval);}

    void    setErrorInfo(int error);
    int     getErrorInfo() { return m_errorInfo;}

    void    setResult(QString result) { m_result = result; emit resultChanged(result);}
    QString getResult() { return m_result; }
    bool    IsOpen()const;
    void    startGetGps(int timeOut = 30);     //单位为秒
    void    stopGetGps();
protected:
signals :
    void    errorInfoChanged(int errorInfo);
    void    resultChanged(QString result);
    void    updateIntervalChanged(int interval);
    void    sigGetGpsCoordinate(double lat, double lon, double alt, int sig = 0);
    void    sigStartGetGps(int timeOut);
    void    sigStopGetGps();
private slots:
    void    sltPositionUpdate(const QGeoPositionInfo &info);
    void    sltUpdateTimeout();
    void    sltGetError(QGeoPositionInfoSource::Error error);

    void    sltSatellitesChanged(const QList<QGeoSatelliteInfo> &infos);
    void    sltSatellitesTimeout();
    void    sltSatellitesError(QGeoSatelliteInfoSource::Error error);
    void    onStartGetGps(int timeOut);
    void    onStopGetGps();
private:
    QGeoPositionInfoSource  *m_PosInfoSource;
    QGeoSatelliteInfoSource *m_SatelliteSource;
    int                     m_errorInfo;
    QString                 m_result;
    int                     m_updateInterval;
    bool                    m_bOpenGps;
    bool                    m_bStart;
    int                     m_countSatellites;
};

#endif // VGGETGPSLOCATION_H
