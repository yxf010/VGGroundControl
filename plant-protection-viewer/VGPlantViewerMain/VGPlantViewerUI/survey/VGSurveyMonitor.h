#ifndef __VGSURVEYMONITOR_H__
#define __VGSURVEYMONITOR_H__

#include <QObject>
#include <QGeoCoordinate>

class VGSurveyMonitor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool connected READ IsConnected NOTIFY connectedChanged)
    Q_PROPERTY(QGeoCoordinate checkCoord READ GetCheckCoord WRITE  SetCheckCoord NOTIFY checkCoordChanged)
    Q_PROPERTY(QGeoCoordinate coordinate READ GetCoordinate NOTIFY coordinateChanged)
    Q_PROPERTY(double lat READ GetLatitude NOTIFY relativeAltitudeChanged)
    Q_PROPERTY(double lon READ GetLongitude NOTIFY relativeAltitudeChanged)
    Q_PROPERTY(double relativeAltitude READ RelativeAltitude NOTIFY relativeAltitudeChanged)
    Q_PROPERTY(double altitude READ Altitude NOTIFY altitudeChanged)
    Q_PROPERTY(double distance READ Distance NOTIFY distanceChanged)
    Q_PROPERTY(int satNum READ GetSatNum NOTIFY satNumChanged)
    Q_PROPERTY(double powerPercent READ PowerPercent NOTIFY powerPercentChanged)
    Q_PROPERTY(double compassAngle READ CompassAngle NOTIFY compassAngleChanged)
    Q_PROPERTY(double rollAngle READ RollAngle NOTIFY rollAngleChanged)
    Q_PROPERTY(double pitchAngle READ PitchAngle NOTIFY pitchAngleChanged)
    Q_PROPERTY(double precision READ Precision NOTIFY precisionChanged)
    Q_PROPERTY(double errRange READ ErrRange NOTIFY errRangeChanged)

    Q_PROPERTY(QString posTip READ GetPositionTip  NOTIFY posTipChanged)
    Q_PROPERTY(bool posValid READ IsPositionValid WRITE SetPositionValid NOTIFY positionValidChanged)
public:
    explicit VGSurveyMonitor(QObject *parent = NULL);
    ~VGSurveyMonitor();
public:
    bool IsPositionValid()const;
    void SetPositionValid(bool bValid);
    void SetLatAndLon(double lat, double lon, bool bHasGGa=false);
    void SetRelativeAltitude(double altitude);
    void SetAltitude(double altitude);
    void SetSatNum(int n);
    void SetPowerPercent(double percent);
    void SetDistance(double distance);
    void SetCompassAngle(double angle);
    void SetPitchAngle(double angle);
    void SetRollAngle(double angle);
    void SetPrecision(double f);
    void setErrRange(double errRange);
    void SetConnected(bool connected);
    void SetPosMode(int mode);
    void SetFixType(int type);
    QGeoCoordinate GetCheckCoord()const;
public:
    static double correctValue(double v, const QString flag);
    static double toGGAValue(double v);
protected:
    //qml property
    QString GetPositionTip()const;
    QGeoCoordinate GetCoordinate()const;
    bool IsConnected()const;
    double RelativeAltitude()const;
    double Altitude()const;
    int GetSatNum()const;
    double PowerPercent()const;
    double Distance()const;
    double CompassAngle()const;
    double RollAngle()const;
    double PitchAngle();
    double Precision()const;
    void SetCheckCoord(const QGeoCoordinate &c);
    double ErrRange() const;
    double GetLatitude()const;
    double GetLongitude()const;
    //overload
    void timerEvent(QTimerEvent *event);
protected slots:
    void onReceiveRtcm(const QList<uint16_t> idxs, const QList<QByteArray> &rtcms);
signals:
    void connectedChanged(bool b);
    void relativeAltitudeChanged(double altitude);
    void altitudeChanged(double altitude);
    void distanceChanged(double distance);
    void satNumChanged(int n);
    void powerPercentChanged(double pow);
    void compassAngleChanged(int angle);
    void rollAngleChanged(int angle);
    void pitchAngleChanged(int angle);
    void precisionChanged(int angle);

    void positionValidChanged(bool isPositionValid);
    void coordinateChanged(const QGeoCoordinate &coor);
    void checkCoordChanged(const QGeoCoordinate &coor);
    void posTipChanged(const QString &tip);
    void errRangeChanged(double errRange);
private:
    void _initial();
private:
    double  m_lat;
    double  m_lon;
    int     m_satNum;
    double  m_relativeAltitude;
    double  m_altitude;
    double  m_distance;
    double  m_compassAngle;
    double  m_rollAngle;
    double  m_pitchAngle;
    double  m_powerPercent;
    double  m_fPrecision;
    double  m_errRange;

    int     m_idTimer;
    int     m_posMode;
    int     m_countTimeout;
    bool    m_isConnected;
    bool    m_isPositionValid;
    QGeoCoordinate m_coorCheck;
};

#endif // __VGSURVEYMONITOR_H__
