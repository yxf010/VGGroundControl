#ifndef __VGLANDINFORMATION_H__
#define __VGLANDINFORMATION_H__

#include <QObject>
#include "MapAbstractItem.h"
#include <QVariant>
#include <QString>
#include <QGeoCoordinate>

#define LandID "surveyId"

//信息类，包括轮廓的经纬度及障碍物
class VGLandBoundary;
class VGLandManager;
class VGCoordinate;
class VGDbManager;
class VGNetManager;
class VGGetGpsLocation;
class VGCoordinate;

class VGLandInformation : public SingleTriggerItem<MapAbstractItem>
{
    Q_OBJECT

    Q_PROPERTY(QString userId READ userId WRITE setUserId NOTIFY userIdChanged)
    Q_PROPERTY(qint64 surveyTime READ surveyTime NOTIFY surveyTimeChanged)
	Q_PROPERTY(int precision READ precision WRITE setPrecision NOTIFY precisionChanged)

    Q_PROPERTY(QString address READ GetAddress WRITE SetAddress NOTIFY addressChanged)
    Q_PROPERTY(QString ownerName READ ownerName WRITE setOwnerName NOTIFY ownerNameChanged)
    Q_PROPERTY(QString ownerBirthday READ ownerBirthday WRITE setOwnerBirthday NOTIFY ownerBirthdayChanged)
    Q_PROPERTY(QString ownerAddr READ ownerAddr WRITE setOwnerAddr NOTIFY ownerAddrChanged)
    Q_PROPERTY(QString ownerPhone READ ownerPhone WRITE setOwnerPhone NOTIFY ownerPhoneChanged)
    Q_PROPERTY(QGeoCoordinate checkCoord READ GetCheckCoord WRITE SetCheckCoord NOTIFY checkCoordChanged)
    Q_PROPERTY(QString actualSurveyId READ actualSurveyId WRITE setActualSurveyId NOTIFY actualSurveyIdIdChanged)
    Q_PROPERTY(SurveyType surveyType READ GetSurveyType WRITE SetSurveyType NOTIFY surveyTypeChanged)//测绘状态
public:
    class OwnerStruct
    {
    public:
        OwnerStruct();
        OwnerStruct(const OwnerStruct &oth);
        QString ToString()const;
        void ParseString(const QString &owner);
    public:
        QString strName;
        qint64  tmBirthday;
        QString strAddress;
        QString strPhone;
    };
public:
    explicit VGLandInformation(QObject *parent = NULL);
    explicit VGLandInformation(const VGLandInformation &info);
    ~VGLandInformation();

    QGeoCoordinate GetCheckCoord()const;
    void SetCheckCoord(const QGeoCoordinate &c);

    QString ownerName() const{ return m_owner.strName; }
    void setOwnerName(const QString &param);

    qint64 GetOwnerBirthday()const;
    QString ownerBirthday()const;
    void setOwnerBirthday(const QString &param);

    QString ownerAddr() const{ return m_owner.strAddress; }
    void setOwnerAddr(const QString &param);

    QString ownerPhone() const{ return m_owner.strPhone; }
    void setOwnerPhone(const QString &param);

    QString GetAddress()const;
    void SetAddress(const QString &addr);

    QString userId() const{ return m_userId; }
    void setUserId(const QString &user);

    void SetSurveyTime(qint64);
    qint64 surveyTime() const{ return m_time; }

    int precision() const{ return m_precision; }
    void setPrecision(int precision);

    bool IsSaveLocal() const{ return m_bSaveLocal; }
    void SetSaveLocal(bool bSl);

    bool IsUpLoaded()const{ return m_bUploaded; }
    void SetUploaded(bool bUpload);

    QString actualSurveyId()const;
    void setActualSurveyId(QString id);

    QList<VGCoordinate*> getCoorsSurvey() {return m_coorsSurvey;}

    MapItemType ItemType()const;
    bool operator==(const MapAbstractItem &li)const;

    void showContent(bool b);
    SurveyType  GetSurveyType() const;

    MapAbstractItem *Clone(QObject *parent)const;

    void SetSelected(bool b);
    void InsertBoundary(VGLandBoundary *bdy, int idx = -1);
    void SetRemoveDB();
    void clearCoors(); 
    bool save(bool bCloud);
    OwnerStruct *owner();
protected:
    //只允许qml调用
    bool                IsSurveyLand()const;
    void                SetSurveyLand(bool b);
    void                SetSurveyType(SurveyType tp);

    void releaseSafe(); 
protected slots:
    void                sltGetGpsCoordinate(double lat, double lon, double alt, int sig);
    void                onContentDestroyed(QObject *obj);
private:
    void                _addOneBoundaryPoint(const QGeoCoordinate &coor, int nSat);
signals:
	void    precisionChanged(int precision);
    void    surveyTimeChanged(qint64 time);
	void    userIdChanged(const QString & user);
	void    coordChanged(const QGeoCoordinate &coor);
    void    addressChanged(const QString &param);
    void    ownerNameChanged(const QString &param);
    void    ownerBirthdayChanged(const QString &param);
	void    ownerAddrChanged(const QString &param);
	void    ownerPhoneChanged(const QString &param);
	void    checkCoordChanged(const QGeoCoordinate &coordinate);
	void    actualSurveyIdIdChanged(const QString &id);
    void    surveyChanged(bool bSurvey);
    void    surveyTypeChanged(SurveyType tp);
private:
    friend  VGDbManager;
    friend  VGNetManager;

    QString                                     m_actualSurveyId;           //地块测绘信息id（服务器）
    QString                                     m_userId;                   //测绘人员id
    QString                                     m_adress;                   //地块地址
    qint64                                      m_time;                     //测绘时间
    OwnerStruct                                 m_owner;
    int                                         m_precision;                //测绘精度

    QList<VGCoordinate*>                        m_coorsSurvey;              //测绘点
    QList<VGLandBoundary*>                      m_boundarys;                //他的地边
    QGeoCoordinate                              m_centerCoordinate;

    bool                                        m_bUploaded;                //是否该地块信息已经上传到云平台
    bool                                        m_bSaveLocal;               //该地块是否保存
    bool                                        m_bSurvey;
    bool                                        m_bRemoveDB;
    SurveyType                                  m_typeSurvey;               //圈地类型
};

#endif // __VGLANDINFORMATION_H__
