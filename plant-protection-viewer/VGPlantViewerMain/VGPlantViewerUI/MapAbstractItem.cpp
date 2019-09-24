#include "MapAbstractItem.h"
#include <QMetaProperty>
#include "MapItemFactory.h"
#include "VGApplication.h"
#include "VGMapManager.h"
#include "VGNetManager.h"
#include "VGGlobalFun.h"
#include "VGDbManager.h"

MapAbstractItem::MapAbstractItem(QObject *parent, int id) : QObject(parent)
, m_id(id), m_bShowBelonged(true)
, m_bVisible(false)
, m_width(2)
, m_modelParent(NULL)
{
    SetParentItem(dynamic_cast<MapAbstractItem*>(parent));
    connect(this, &MapAbstractItem::selectedChanged, this, &MapAbstractItem::onSelectedChanged);
}

MapAbstractItem::MapAbstractItem(const MapAbstractItem &oth) : QObject(NULL)
, m_id(oth.m_id)
, m_bVisible(oth.m_bVisible)
, m_color(oth.m_color)
, m_width(oth.m_width)
, m_parentItem(NULL)
, m_modelParent(NULL)
, m_bShowBelonged(true)
{
    connect(this, &MapAbstractItem::selectedChanged, this, &MapAbstractItem::onSelectedChanged);
}

MapAbstractItem::~MapAbstractItem()
{
    if (m_modelParent)
        m_modelParent->removeOne(this);
}

void MapAbstractItem::SetColor(const QColor &)
{
}

QColor MapAbstractItem::GetColor() const
{
    return Qt::transparent;
}

void MapAbstractItem::SetBorderColor(const QColor &col)
{
    if (m_color != col)
    {
        m_color = col;
        emit borderColorChanged(col);
    }
}

QColor MapAbstractItem::GetBorderColor() const
{
    return m_color;
}

QColor MapAbstractItem::GetItemColor() const
{
	return QColor();
}

bool MapAbstractItem::GetSelected() const
{
    if (VGMapManager *mgr = qvgApp->mapManager())
        return mgr->IsItemSelecte(this);

    return false;
}

void MapAbstractItem::SetSelected(bool b)
{
    if (b == GetSelected())
        return;

    if (VGMapManager *mgr = qvgApp->mapManager())
    {
        mgr->SetItemSelected(this, b);
        if (b)
            mgr->InsertItem(this);
    }
    showContent(b);
    emit itemColorChanged(GetItemColor());
    emit selectedChanged(b);
}

void MapAbstractItem::SetVisible(bool b)
{
    if (m_bVisible != b)
    {
        m_bVisible = b;
        emit visibleChanged(b);
    }
}

bool MapAbstractItem::GetVisible() const
{
    if (m_parentItem)
        return m_bVisible && m_parentItem->GetVisible();

    return m_bVisible;
}

int MapAbstractItem::GetId() const
{
    return m_id;
}

void MapAbstractItem::SetId(int id)
{
    if (m_id != id)
    {
        m_id = id;
        emit idChanged(id);
    }
}

int MapAbstractItem::GetWidth() const
{
    return m_width;
}

void MapAbstractItem::SetWidth(int w)
{
    if (m_width != w)
    {
        m_width = w;
        emit widthChanged(w);
    }
}

void MapAbstractItem::Show(bool b)
{
    _show(b);
    showContent(b);
}

void MapAbstractItem::showContent(bool)
{
}

int MapAbstractItem::ItemIndex(const MapAbstractItem *) const
{
    return -1;
}

bool MapAbstractItem::save(bool bCloud)
{
    if (!bCloud)
    {
        VGDbManager *db = qvgApp->dbManager();
        if (!db)
            return false;

        db->SaveItemSafe(this);
        return true;
    }
    else if (VGNetManager *netMgr = qvgApp->netManager())
    {  
        connect(netMgr, &VGNetManager::sigSaveResult, this, &MapAbstractItem::onUploadChanged);
        return netMgr->UploadItem(this);
    }
    return false;
}

bool MapAbstractItem::CompareItemList(const QList<MapAbstractItem*> &ls1, const QList<MapAbstractItem*> &ls2)
{
    if (ls1.count() != ls2.count())
        return false;

    QList<MapAbstractItem*>::const_iterator itr = ls1.begin();
    QList<MapAbstractItem*>::const_iterator itr2 = ls2.begin();
    for (; itr != ls1.end(); ++itr, ++itr2)
    {
        if (**itr != **itr2)
            return false;
    }

    return true;
}

bool MapAbstractItem::IsContain(const DescribeMap &m1, const DescribeMap &m2)
{
    DescribeMap::const_iterator itr = m2.begin();
    for (; itr != m2.end(); ++itr)
    {
        if (itr.value() != m1[itr.key()])
            return false;
    }
    return true;
}

void MapAbstractItem::onUploadChanged(const DescribeMap &result)
{
    if (VGNetManager *netMgr = qobject_cast<VGNetManager *>(sender()))
    {
        netMgr->disconnect(SIGNAL(sigSaveResult(const DescribeMap &)), this);
        processSaveReslt(result);
    }
}

bool MapAbstractItem::operator!=(const MapAbstractItem &item) const
{
    return !(*this == item);
}

MapAbstractItem * MapAbstractItem::Clone(QObject *parent) const
{
    if (MapAbstractItem *ret = MapItemFactory::CreateItem(metaObject()->className(), parent))
    {
        *ret = *this;
        return ret;
    }

    return NULL;
}

int MapAbstractItem::CountChildren() const
{
    return 0;
}


int MapAbstractItem::GetShowType() const
{
    return Show_LineAndPoint;
}

void MapAbstractItem::SetShowType(int)
{
}

void MapAbstractItem::SetParentItem(MapAbstractItem *parent)
{
    m_parentItem = parent;
    setParent(parent);
}

MapAbstractItem *MapAbstractItem::GetParentItem() const
{
    return m_parentItem;
}

MapAbstractItem * MapAbstractItem::childAt(int)const
{
    return NULL;
}

void MapAbstractItem::releaseSafe()
{
    if (VGDbManager *db = qvgApp->dbManager())
        db->DeleteItemSafe(this);
    if (VGNetManager *nm = qvgApp->netManager())
        nm->DeleteItemSafe(this);

    SetVisible(false);
    deleteLater();
}

void MapAbstractItem::foldChildren(bool)
{
}

void MapAbstractItem::_show(bool b)
{
    if (VGMapManager *mgr = qvgApp->mapManager())
    {
        if (b)
        {
            int idx = m_parentItem ? m_parentItem->ItemIndex(this) : -1;
            if (idx >= 0)
                --idx;
            mgr->InsertItem(this, idx);
        }
        else
        {
            mgr->RemoveItem(this);
            m_modelParent = NULL;
        }
    }
}

MapAbstractItem &MapAbstractItem::operator=(const MapAbstractItem &item)
{
    const QMetaObject *mo = item.metaObject();
    for (int i = 0; i < mo->propertyCount(); ++i)
    {
        QMetaProperty p = mo->property(i);
        if (!p.isWritable())
            continue;

        setProperty(p.name(), p.read(&item));
    }

    return *this;
}

void MapAbstractItem::processSaveReslt(const DescribeMap &)
{
}

void MapAbstractItem::SetParentModel(QmlObjectListModel *model)
{
    m_modelParent = model;
}

bool MapAbstractItem::IsShowBelonged() const
{
    return m_bShowBelonged;
}

void MapAbstractItem::SetShowBelonged(bool b)
{
    if (b == m_bShowBelonged)
        return;

    m_bShowBelonged = b;
    if (m_parentItem)
        m_parentItem->showContent(b);

    emit showBelongedChanged(b);
}

MapAbstractItem *MapAbstractItem::FindParentByProperty(const DescribeMap &)const
{
    return NULL;
}

void MapAbstractItem::UpdateDB()const
{
    if (VGDbManager *db = qvgApp->dbManager())
        db->UpdateDb(this);
}

void MapAbstractItem::SetUploaded(bool)
{
}

bool MapAbstractItem::IsUpLoaded() const
{
    return false;
}

void MapAbstractItem::SetSaveLocal(bool)
{
}

bool MapAbstractItem::IsSaveLocal() const
{
    return false;
}

void MapAbstractItem::onSelectedChanged(bool)
{
}
///////////////////////////////////////////////////////////////////////////////
//MapAbstractItem
///////////////////////////////////////////////////////////////////////////////
MapListItem::MapListItem(QObject *parent, int id) : MapAbstractItem(parent, id)
{
}

MapListItem::MapListItem(const MapListItem &oth) : MapAbstractItem(oth)
{
}

MapListItem::~MapListItem()
{
}

int MapListItem::CountChildren() const
{
    return m_children.count();
}

MapAbstractItem * MapListItem::childAt(int i)const
{
    if (i < 0 || i >= m_children.count())
        return NULL;

    return m_children.at(i);
}

MapAbstractItem *MapListItem::TakeAt(int idex)
{
    if (idex >= m_children.count() || idex < 0)
        return NULL;

    MapAbstractItem *ret = m_children.takeAt(idex);
    ret->m_parentItem = NULL;
    return ret;
}

void MapListItem::Insert(MapAbstractItem *item, int idex)
{
    if (!item)
        return;

    if (idex > m_children.count() || idex < 0)
        m_children.append(item);
    else
        m_children.insert(idex, item);

    item->SetParentItem(this);
    connect(item, SIGNAL(destroyed(QObject *)), this, SLOT(onChildDestroyed(QObject *)));
}

void MapListItem::Remove(int idex)
{
    if (idex >= m_children.count() || idex < 0)
        return;
    if (MapAbstractItem *item = m_children.takeAt(idex))
        disconnect(item, SIGNAL(destroyed(QObject *)), this, SLOT(onChildDestroyed(QObject *)));
}

void MapListItem::Remove(MapAbstractItem *item)
{
    if (m_children.contains(item))
    {
        m_children.removeAll(item);
        disconnect(item, SIGNAL(destroyed(QObject *)), this, SLOT(onChildDestroyed(QObject *)));
    }
}

int MapListItem::Indexof(MapAbstractItem *item) const
{
    return m_children.indexOf(item);
}

void MapListItem::foldChildren(bool b)
{
    if (VGMapManager *mgr = qvgApp->mapManager())
    {
        foreach(MapAbstractItem *itr, m_children)
        {
            if (b)
                mgr->InsertItem(itr);
            else
                itr->Show(false);
        }
    }
}

MapAbstractItem *MapListItem::findChildItem(const DescribeMap &)const
{
    return NULL;
}

void MapListItem::releaseSafe()
{
    foreach(MapAbstractItem *itr, m_children)
    {
        itr->releaseSafe();
    }

    MapAbstractItem::releaseSafe();
}

bool MapListItem::operator==(const MapAbstractItem &item) const
{
    const MapListItem *ls = dynamic_cast<const MapListItem*>(&item);
    if (!ls)
        return false;
    if (!ls || CountChildren() != ls->CountChildren())
        return false;
    return CompareItemList(m_children, ls->m_children);
}

MapAbstractItem *MapListItem::Clone(QObject *parent) const
{
    const QMetaObject *mo = metaObject();
    MapListItem *ret = (MapListItem *)MapItemFactory::CreateItem(mo->className(), parent);
    if (!ret)
        return ret;

    for (int i = 0; i < mo->propertyCount(); ++i)
    {
        QMetaProperty p = mo->property(i);
        if (!p.isWritable())
            continue;

        ret->setProperty(p.name(), p.read(this));
    }

    foreach(MapAbstractItem *itr, m_children)
    {
        ret->m_children.push_back(itr->Clone());
    }

    return ret;
}

void MapListItem::onChildDestroyed(QObject *obj)
{
    if (obj)
        Remove((MapAbstractItem*)obj);
}
