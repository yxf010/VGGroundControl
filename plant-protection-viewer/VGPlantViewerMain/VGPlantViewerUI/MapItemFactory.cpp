#include "MapItemFactory.h"
#include <QString>

static QMap<QString, MapAbstractFactoryItem*> &FactoryMap()
{
    static QMap<QString, MapAbstractFactoryItem*> sMap;
    return sMap;
}
////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////
MapAbstractFactoryItem::MapAbstractFactoryItem(const QString &name) :m_name(name)
{
    Register(name, this);
}


MapAbstractFactoryItem::~MapAbstractFactoryItem()
{
    Unregister(m_name);
}


void MapAbstractFactoryItem::Register(const QString &name, MapAbstractFactoryItem *item)
{
    QMap<QString, MapAbstractFactoryItem*> fi = FactoryMap();
    if (fi.find(name) != fi.end())
        return;

    fi[name] = item;
}


void MapAbstractFactoryItem::Unregister(const QString &name)
{
    QMap<QString, MapAbstractFactoryItem*> fi = FactoryMap();
    if (fi.find(name) == fi.end())
        return;

    fi.remove(name);
}
////////////////////////////////////////////////////////////////////////////////////
//MapItemFactory
////////////////////////////////////////////////////////////////////////////////////
MapAbstractItem * MapItemFactory::CreateItem(const QString &clsName, QObject *parent)
{
    QMap<QString, MapAbstractFactoryItem*> fi = FactoryMap();
    QMap<QString, MapAbstractFactoryItem*>::Iterator itr = fi.find(clsName);
    if (itr != fi.end())
        return (*itr)->CreateItem(parent);

    return NULL;
}
