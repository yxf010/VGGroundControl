#ifndef __MAPABSTRACTITEM_H__
#define __MAPABSTRACTITEM_H__

#include <QObject>
#include <QColor>
#include <QList>
#include <QMetaObject>
#include <QMap>

#define SAVE_SUCCESS "success"
#define RELEASEPOINTER(pt)\
{delete pt; pt=NULL;}

class QmlObjectListModel;
class MapListItem;
typedef QMap<QString, QVariant> DescribeMap;

//地图上显示对象的抽象
class MapAbstractItem : public QObject
{
    Q_OBJECT

    Q_ENUMS(SurveyType)
    Q_ENUMS(ShowType)
    Q_ENUMS(MapItemType)
    Q_ENUMS(Oriente)
    Q_PROPERTY(QColor color READ GetColor WRITE SetColor NOTIFY colorChanged)
	Q_PROPERTY(QColor borderColor READ GetBorderColor WRITE SetBorderColor NOTIFY borderColorChanged)
	Q_PROPERTY(QColor itemColor READ GetItemColor NOTIFY itemColorChanged)
	Q_PROPERTY(bool visible READ GetVisible WRITE SetVisible NOTIFY visibleChanged)
	Q_PROPERTY(bool selected READ GetSelected WRITE SetSelected NOTIFY selectedChanged)
    Q_PROPERTY(int width READ GetWidth  WRITE SetWidth NOTIFY widthChanged)
    Q_PROPERTY(int id READ GetId WRITE SetId NOTIFY idChanged)
    Q_PROPERTY(int showType READ GetShowType WRITE SetShowType)
    Q_PROPERTY(MapItemType itemType READ ItemType CONSTANT)
    Q_PROPERTY(int countChildren READ CountChildren CONSTANT)
    Q_PROPERTY(bool saveLocal READ IsSaveLocal NOTIFY saveLocalChanged)
    Q_PROPERTY(bool uplaoded READ IsUpLoaded NOTIFY uplaodedChanged)
    Q_PROPERTY(bool showBelonged READ IsShowBelonged WRITE SetShowBelonged NOTIFY showBelongedChanged)
public:
    enum ShowType
    {
        Show_Line = 1,
        Show_Point = 2,
        Show_LineAndPoint = 3,
    };
    enum Oriente
    {
        East = 0,
        South,
        West,
        North,
    };
    enum SurveyType
    {
        Survey_No,                //没有测绘
        Survey_DrawMap,           //地图描点
        Survey_GPS,               //手机GPS轨迹
        Survey_Vehicle,            //测绘仪
    };
    enum MapItemType
    {
        Type_Unknow = 0,
        Type_BaseBeg,
        Type_SupportPoint = Type_BaseBeg,
        Type_Point,
        Type_DashLine,
        Type_PolyLine,
        Type_Polygon,
        Type_Route,
        Type_Home,
        Type_ContinueFly,
        Type_BoundaryPoint,
        Type_SequencePoint,

        Type_BaseEnd = 50,

        Type_LandInfo = Type_BaseEnd,
        Type_LandBoundary,
        Type_FlyRoute,
        Type_PlantInfo,
        Type_ViewItem,
        Type_RouteTip,
        Type_APoint,
        Type_BPoint,
        Type_OperateMission,
        Type_VehicleMission,
        Type_PointSelect,
        Type_SelectEdge,
    };
public:
    explicit MapAbstractItem(QObject *parent = NULL, int = -1);
    MapAbstractItem(const MapAbstractItem&);
    ~MapAbstractItem();

    virtual void SetColor(const QColor &col);
    virtual QColor GetColor()const;
    void SetBorderColor(const QColor &col);
    QColor GetBorderColor()const;
    bool GetSelected()const;
    virtual void SetSelected(bool);

    virtual QColor GetItemColor()const;

    void SetVisible(bool b);
    bool GetVisible()const;

    int GetId()const;
    void SetId(int id);

    int GetWidth()const;
    void SetWidth(int id);

    virtual int GetShowType()const;
    virtual void SetShowType(int t);
    virtual void SetParentItem(MapAbstractItem *parent);
    MapAbstractItem *GetParentItem()const;

    virtual void Show(bool b);
    bool operator!=(const MapAbstractItem &item)const;
    virtual MapAbstractItem *Clone(QObject *parent=NULL)const;
    virtual int CountChildren()const;

    virtual MapItemType ItemType()const = 0;
    virtual bool operator==(const MapAbstractItem &item)const = 0;
    MapAbstractItem &operator=(const MapAbstractItem&);

    Q_INVOKABLE virtual void showContent(bool b);
    Q_INVOKABLE virtual bool save(bool bCloud);
    virtual int ItemIndex(const MapAbstractItem *item)const;
    virtual bool IsSaveLocal()const;
    virtual MapAbstractItem *FindParentByProperty(const DescribeMap &)const;
    void UpdateDB()const;
public:
    static bool CompareItemList(const QList<MapAbstractItem*> &ls1, const QList<MapAbstractItem*> &ls2);
    static bool IsContain(const DescribeMap &m1, const DescribeMap &m2);
public slots:
    void onUploadChanged(const DescribeMap &result);
protected:
    virtual void processSaveReslt(const DescribeMap &result);
    void SetParentModel(QmlObjectListModel *model);
    bool IsShowBelonged()const;
    void SetShowBelonged(bool b);
    virtual void SetSaveLocal(bool);
    virtual bool IsUpLoaded()const;
    virtual void SetUploaded(bool);
    Q_INVOKABLE virtual void releaseSafe();   //qml界面释放
    Q_INVOKABLE virtual MapAbstractItem *childAt(int)const;
    Q_INVOKABLE virtual void foldChildren(bool b);
    void _show(bool b);
signals:
	void colorChanged(const QColor&);
	void borderColorChanged(const QColor&);
	void itemColorChanged(const QColor&);
	void visibleChanged(bool);
	void idChanged(int);
	void widthChanged(int);
	void selectedChanged(bool);
    void saveLocalChanged(bool b);
    void uplaodedChanged(bool b);
    void showBelongedChanged(bool b);
protected slots:
    virtual void onSelectedChanged(bool b);
protected:
    friend  QmlObjectListModel;
    friend  MapListItem;

    int     m_id;
    bool    m_bVisible;
    bool    m_bShowBelonged;
    QColor  m_color;
    int     m_width;
    MapAbstractItem     *m_parentItem;
    QmlObjectListModel  *m_modelParent;
};

class MapListItem : public MapAbstractItem
{
    Q_OBJECT
public:
    explicit MapListItem(QObject *parent = NULL, int = -1);
    explicit MapListItem(const MapListItem &oth);
    ~MapListItem();

    int CountChildren()const;
    MapAbstractItem *childAt(int i)const;
    MapAbstractItem *TakeAt(int i);
    void Insert(MapAbstractItem *item, int idex=-1);
    void Remove(int idex);
    void Remove(MapAbstractItem *item);
    int Indexof(MapAbstractItem *)const;
    bool operator==(const MapAbstractItem &item)const;

    void foldChildren(bool b);
    virtual MapAbstractItem *findChildItem(const DescribeMap &dsc)const;
    void releaseSafe();
protected:
    virtual MapAbstractItem *Clone(QObject *parent = NULL)const;
protected slots:
    virtual void onChildDestroyed(QObject *obj);
protected:
    QList<MapAbstractItem *>    m_children;
};

template<class T = MapAbstractItem>
class SingleTriggerItem : public T
{
public:
	SingleTriggerItem(QObject *parent = NULL, int id = -1) :T(parent, id)
        , m_colorNormal(246, 246, 246), m_colorSelected(205, 230, 255){}
    SingleTriggerItem(const SingleTriggerItem<T> &oth) :T(oth)
        , m_colorNormal(oth.m_colorNormal), m_colorSelected(oth.m_colorSelected){}

	void SetItemColor(const QColor &nor, const QColor sel)
	{
		m_colorNormal = nor;
		m_colorSelected = sel;
	}
	QColor GetItemColor() const
	{
        return T::GetSelected() ? m_colorSelected : m_colorNormal;
	}
protected:
	QColor		m_colorNormal;        //作为listItem颜色
	QColor		m_colorSelected;      //作为listItem选中颜色
};

#endif // VGOUTLINE_H
