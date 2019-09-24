#ifndef __VGVEHICLE_MISSION_H__
#define __VGVEHICLE_MISSION_H__

#include "MapAbstractItem.h"
#include <QGeoCoordinate>
#include <QVariant>

class MissionItem;
class VGFlyRoute;
class VGLandBoundary;
class VGCoordinate;
class VGOutline;
class VGLandInformation;
class VGSupportPolyline;

class VGMissionItem :public MapAbstractItem
{
    Q_OBJECT

    Q_PROPERTY(quint16 command READ GetCommand WRITE SetCommand NOTIFY commandChanged)
    Q_PROPERTY(quint16 frame READ GetFrame WRITE SetFrame NOTIFY frameChanged)
    Q_PROPERTY(int sequence READ GetSequence CONSTANT)
    Q_PROPERTY(QVariant param1 READ GetParam1 WRITE SetParam1 NOTIFY param1Changed)
    Q_PROPERTY(QVariant param2 READ GetParam2 WRITE SetParam2 NOTIFY param2Changed)
    Q_PROPERTY(QVariant param3 READ GetParam3 WRITE SetParam3 NOTIFY param3Changed)
    Q_PROPERTY(QVariant param4 READ GetParam4 WRITE SetParam4 NOTIFY param4Changed)
    Q_PROPERTY(QGeoCoordinate coordinate READ GetCoordinate WRITE SetCoordinate NOTIFY coordinateChanged)
public:
    VGMissionItem(const MissionItem &item, QObject *parent = NULL);
    VGMissionItem(MissionItem *ref, QObject *parent = NULL, int id=0);
    ~VGMissionItem();

    QColor GetColor()const;
    void SetColor(const QColor &col);
    MapItemType ItemType()const;

    quint16 GetCommand()const;
    void SetCommand(quint16 cmd);
    quint16 GetFrame()const;
    void SetFrame(quint16 fr);
    int GetSequence()const;
    void SetSequence(int seq);
    QVariant GetParam1()const;
    void SetParam1(const QVariant &p);
    QVariant GetParam2()const;
    void SetParam2(const QVariant &p);
    QVariant GetParam3()const;
    void SetParam3(const QVariant &p);
    QVariant GetParam4()const;
    void SetParam4(const QVariant &p);
    QGeoCoordinate GetCoordinate()const;
    void SetCoordinate(const QGeoCoordinate &c);
    void SetRelativeAtitude(double h);
    bool operator==(const MapAbstractItem &item)const;
    MissionItem *GetMissionItem()const;
private:
signals :
    void commandChanged(quint16 cmd);
    void frameChanged(quint16 f);
    void coordinateChanged(const QGeoCoordinate&coor);
    void param1Changed(const QVariant &param);
    void param2Changed(const QVariant &param);
    void param3Changed(const QVariant &param);
    void param4Changed(const QVariant &param);
private:
    MissionItem *m_item;
    QColor      m_color;
};

class VGVehicleMission : public MapAbstractItem
{
    Q_OBJECT

    Q_PROPERTY(QVariantList path READ GetPath NOTIFY pathChanged)
    Q_PROPERTY(int countItem READ CountItems NOTIFY countItemChanged)
    Q_PROPERTY(int begin READ GetBegin WRITE SetBegin NOTIFY beginChanged)
    Q_PROPERTY(int end READ GetEnd WRITE SetEnd NOTIFY endChanged)
    Q_PROPERTY(double opHeight READ GetOpHeight WRITE SetOpHeight NOTIFY opHeightChanged)
    Q_PROPERTY(float medPerAcre READ GetMedPerAcre WRITE SetMedPerAcre NOTIFY medPerAChanged)
    Q_PROPERTY(int count READ _countOperationLine CONSTANT)
    Q_PROPERTY(bool hasEnterSup READ HasEnterSup NOTIFY hasEnterSupChanged)
    Q_PROPERTY(bool hasReturnSup READ HasReturnSup NOTIFY hasReturnSupChanged)
    Q_PROPERTY(double enterHeight READ GetEnterHeight WRITE SetEnterHeight NOTIFY enterHeightChanged)
    Q_PROPERTY(double returnHeight READ GetReturnHeight WRITE SetReturnHeight NOTIFY returnHeightChanged)
    Q_PROPERTY(double speed READ GetSpeed WRITE SetSpeed NOTIFY speedChanged)
    Q_PROPERTY(double length READ GetLength NOTIFY lengthChanged)
    Q_PROPERTY(VGLandInformation* curland READ GetLandInformation CONSTANT)
public:
    explicit VGVehicleMission(QObject *parent = NULL, const QList<MissionItem*> &items = QList<MissionItem*>());
    explicit VGVehicleMission(VGFlyRoute *fr);

    void SetMissionItems(const QList<MissionItem*> &items, bool bRef=false);
    const QList<VGMissionItem*> &VGVehicleMissionItems()const;
    QList<MissionItem*> MissionItems()const;
    QList<MissionItem*> BoundaryItems()const;
    void showContent(bool b);
    QVariantList GetPath()const;
    void Show(bool b);
	void SetExecutable(bool b);
	bool GetExecutable()const;

    void Monitor();
    bool operator==(const MapAbstractItem &item)const;
    MapItemType ItemType()const;
    VGFlyRoute *GetFlyRoute()const;
    void SetCurrentExecuteItem(int idx);
    int GetCurrentExecuteItem()const;
    void SetLastExecuteItem(int idx);
    void showSquences(bool b);
    void AtachByPlant();
    void SetSelected(bool);
    uint32_t CountBlock()const;
    double GetSpeed()const;
    VGCoordinate *GetSupportEnter()const;
    VGCoordinate *GetSupportReturn()const;
    QString GetInfo()const;
    Q_INVOKABLE void clearSupport(bool bEnter = true, bool bRcv = false);
    Q_INVOKABLE void addSupport(const QGeoCoordinate &coor, bool bEnter = true, bool bRcv = false);
    VGLandBoundary *GetBelongBoundary()const;
    float MaxWorkAlt()const;
    double GetOpHeight()const;
public:
    static VGVehicleMission *fromInfo(const QString &info);
protected:
    Q_INVOKABLE bool canSync2Vehicle()const;
    Q_INVOKABLE void sync2Vehicle();

    int GetBegin()const;
    void SetBegin(int beg);
    int GetEnd()const;
    void SetEnd(int end);
    void SetOpHeight(double f);
    float GetMedPerAcre()const;
    void SetMedPerAcre(float f);
    bool HasEnterSup()const;
    bool HasReturnSup()const;
    double GetEnterHeight()const;
    void SetEnterHeight(double f);
    double GetReturnHeight()const;
    void SetReturnHeight(double f);
    double GetLength()const;
    VGLandInformation *GetLandInformation()const;
    void processSaveReslt(const DescribeMap &);
    int CountItems()const;
    void SetSpeed(double f);
signals:
    void pathChanged(const QVariantList &path);
    void beginChanged(int beg);
    void endChanged(int end);
    void opHeightChanged(double f);
    void medPerAChanged(double f);
    void hasEnterSupChanged(bool b);
    void hasReturnSupChanged(bool b);
    void enterHeightChanged(double f);
    void returnHeightChanged(double f);
    void lengthChanged(double f);
    void countItemChanged();
    void speedChanged();
protected slots:
    void onItemDestoyed(QObject *obj);
private:
    void _generateMission(VGFlyRoute *fr);
    void _genMissionItem(const QList<VGCoordinate*> &coors, double tm=.1, double r=.5);
    int  _countOperationLine()const;
    int  _getLinePntIndex(int nline, bool bStart = true);
    void _adjustSequence(int beg=0);
    void _supportSegChanged(bool bEnter);
    void _calculatLength();
    void _genTipPoint(VGOutline *ol, int beg, int end);
    void _sendSupport(bool bEnter, VGCoordinate *pnt);
    bool _checkSupport(const QGeoCoordinate &c);
    bool _checkSupportHeight(double f);
private:
    QList<VGMissionItem*>    m_missionItems;
    QVariantList             m_path;
    QList<double>            m_boundarys;
	bool					 m_bExecute;
    VGCoordinate             *m_supportEnter;
    VGCoordinate             *m_supportReturn;
    int                      m_beg;
    int                      m_end;
    VGCoordinate             *m_begTip;
    VGCoordinate             *m_endTip;
    double                   m_opHeight;
    double                   m_enterHeight;
    double                   m_returnHeight;
    double                   m_speed;
    double                   m_length;
    VGSupportPolyline        *m_segEnter;
    VGSupportPolyline        *m_segReturn;
    int                     m_iCurExecuteItem;
    MissionItem             *m_return;
    bool                    m_bShowSeq;
};

#endif // __VGVEHICLE_MISSION_H__
