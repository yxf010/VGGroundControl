#ifndef __MAPITEMFACTORY_H__
#define __MAPITEMFACTORY_H__

#include "MapAbstractItem.h"

//地图上显示对象的抽象
class MapAbstractFactoryItem
{
public:
    MapAbstractFactoryItem(const QString &name);
    virtual ~MapAbstractFactoryItem();

    void Register(const QString &name, MapAbstractFactoryItem *item);
    void Unregister(const QString &name);

    virtual MapAbstractItem *CreateItem(QObject *parent)=0;
private:
    QString m_name;
};

template<class Item>
class MapFactoryItem : public MapAbstractFactoryItem
{
public:
    MapFactoryItem(const QString &name) :MapAbstractFactoryItem(name){}
protected:
    MapAbstractItem *CreateItem(QObject *parent)
    {
        return new Item(parent);
    }
};

namespace MapItemFactory
{
    MapAbstractItem *CreateItem(const QString &clsName, QObject *parent);
};

#define DECLARE_ITEM_FACTORY(className) static MapFactoryItem<className> s_factoryItem_##className(#className);

#endif // VGOUTLINE_H
