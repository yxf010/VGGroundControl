#ifndef __VGLANDBOUNDARY_H__
#define __VGLANDBOUNDARY_H__

#include "MapAbstractItem.h"
#include <QGeoCoordinate>
#include "VGLandInformation.h"

#define  BoundaryId "boundaryId"

//地边
class VGFlyRoute;
class VGCoordinate;
class VGLandPolygon;
class VGDbManager;
class VGLandManager;

class VGLandBoundary : public SingleTriggerItem<MapAbstractItem>
{
    Q_OBJECT

    Q_PROPERTY(QString address READ GetAddress WRITE SetAddress NOTIFY addressChanged)
    Q_PROPERTY(QString describe READ GetDescribe WRITE SetDescribe NOTIFY describeChanged)
    Q_PROPERTY(QString statDescribe READ GetStatDescribe WRITE SetStatDescribe)
    Q_PROPERTY(int countBlocks READ BlocksCount NOTIFY countBlocksChanged)
    Q_PROPERTY(QString surveyDescribe READ GetSurveyDescribe CONSTANT)
    Q_PROPERTY(SurveyType surveyType READ GetSurveyType CONSTANT)
    Q_PROPERTY(QString user READ GetUser CONSTANT)
    Q_PROPERTY(QString owner READ GetOwnerName CONSTANT)
    Q_PROPERTY(QString ownerPhone READ GetOwnerPhone CONSTANT)
    Q_PROPERTY(qint64 editTime READ GetTime NOTIFY editTimeChanged)
    Q_PROPERTY(double landArea READ GetLandArea NOTIFY landAreaChanged)
    Q_PROPERTY(QGeoCoordinate center READ GetCenter NOTIFY centerChanged)
    Q_PROPERTY(int curBlockIndex READ GetCurBlockIndex WRITE SetCurBlockIndex NOTIFY curBlockChanged)
    Q_PROPERTY(bool valid READ IsValid  NOTIFY validChanged)
    Q_PROPERTY(QString boundaryId READ GetBoundaryId NOTIFY boundaryIdChanged)
public:
    explicit VGLandBoundary(QObject *info = NULL);
    VGLandBoundary(const VGLandBoundary &oth);
    ~VGLandBoundary(); 

    VGLandInformation *GetBelongedLand()const;
    void SetBelongedLand(VGLandInformation *);
    MapItemType ItemType()const;
    bool operator==(const MapAbstractItem &item)const;
    void showContent(bool b);
    void SetShowType(int t);

    VGLandPolygon *GetBoundaryPolygon()const;
    QString GetDescribe()const;
    void SetDescribe(const QString &str);

    qint64 GetTime()const;
    MapAbstractItem *Clone(QObject *parent)const;
    const QList<VGLandPolygon*> &Polygos()const;
    void SetSelected(bool b);
    QGeoCoordinate GetCenter()const;
    void releaseSafe();
    bool IsValid()const;
    const QList<double> &GetBoundarys();

    int BlocksCount()const;
    QString GetSurveyDescribe()const;
    QString GetUser()const;
    QString GetOwnerName()const;
    QString GetOwnerPhone()const;
    double GetLandArea()const;
    int GetPrecision()const;
    QString GetBoundaryId()const;
    VGLandInformation::OwnerStruct *owner();
    QString GetLandActId()const;
    bool save(bool bCloud);
    bool IsSaveLocal() const;
protected:
    void SetBoundaryId(const QString &str);
    QString GetAddress()const; 
    void SetAddress(const QString &str);
    QString GetStatDescribe()const;
	void SetStatDescribe(const QString &str);

	void SetSaveLocal(bool bSl);
	bool IsUpLoaded()const;
    void SetUploaded(bool bUpload);

    int GetCurBlockIndex()const;
    void SetCurBlockIndex(int i);

    //qml编辑有关
    Q_INVOKABLE void addBoudaryPoint(const QGeoCoordinate &coor, VGCoordinate *c=NULL);
    Q_INVOKABLE void addBlockPoint(const QGeoCoordinate &coor, VGCoordinate *c = NULL);
    Q_INVOKABLE void addBlock();
    Q_INVOKABLE void clearBoudary();
    Q_INVOKABLE void clearSurvey();
    Q_INVOKABLE VGLandBoundary *adjust();

    VGLandManager *GetLandManager()const;
    SurveyType GetSurveyType()const;
    void processSaveReslt(const DescribeMap &result);
    MapAbstractItem *FindParentByProperty(const DescribeMap &dsc)const;
signals:
    void describeChanged(const QString &str);
    void countBlocksChanged(); 
    void landAreaChanged(double a);
    void centerChanged(const QGeoCoordinate &coor); 
    void editTimeChanged();
    void validChanged(bool bValid);
    void curBlockChanged(int idx);
    void editFinished(VGLandBoundary *);
    void boundaryIdChanged(const QString &str);
    void addressChanged(const QString &str);
protected slots:
    void onPlanRouteFinished();
    void onBoundaryChange();
    void onBoundaryCoorsChanged(int count);
private:
    bool _isBoundaryEdited()const;
    void _addSafeAreaPoint(const QGeoCoordinate &coor);
    void _calcArea();
    VGLandPolygon *_addBoundary(int tp);
    bool _isLastBlockValid()const;
    VGLandPolygon *_getCurBlock()const;
private:
    friend                  VGFlyRoute;
    friend                  VGDbManager;
    friend                  VGNetManager;

    QString                 m_cloudID;          //服务器上ID
    QList<VGLandPolygon*>   m_polygons;         //边界图
    VGLandPolygon           *m_safeArea;        //安全区
    QList<double>           m_boundarys;

    //VGCoordinate      *m_APoint;
    bool                m_bSaveLocal;
    bool                m_bUploaded;
    bool                m_bSafeEdited;
    QString             m_strDescribe;
    QString             m_strStatDescribe;
    qint64              m_time;
    double              m_area;
    QGeoCoordinate      m_center;
    int                 m_curBlock;
};

#endif // VGLANDINFORMATION_H
