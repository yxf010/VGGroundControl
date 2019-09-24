#include "QmlObjectListModel.h"
#include "MapAbstractItem.h"

#include <QDebug>
#include <QQmlEngine>

#define ObjectRole  Qt::UserRole
#define TextRole  (Qt::UserRole + 1)

QmlObjectListModel::QmlObjectListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QmlObjectListModel::QmlObjectListModel(QmlObjectListModel &model)
    : QAbstractListModel(model.parent())
    , m_lsObject(model.m_lsObject)
{
}

QmlObjectListModel::~QmlObjectListModel()
{
    for (QObject *obj : m_lsObject)
    {
		if (MapAbstractItem *item = dynamic_cast<MapAbstractItem *>(obj))
			item->SetParentModel(NULL);
    }
}

QObject *QmlObjectListModel::get(int index) const
{
    if (index<0 || index>m_lsObject.count())
        return NULL;

    return m_lsObject.at(index);
}

int QmlObjectListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    
    return m_lsObject.count();
}

QVariant QmlObjectListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    
    if (index.row() >= m_lsObject.count()) {
        return QVariant();
    }
    
    if (role == ObjectRole)
        return QVariant::fromValue(m_lsObject[index.row()]);
    if (role == TextRole)
        return QVariant::fromValue(m_lsObject[index.row()]->objectName());

    return QVariant();
}

QHash<int, QByteArray> QmlObjectListModel::roleNames(void) const
{
    QHash<int, QByteArray> hash;
    
    hash[ObjectRole] = "object";
    hash[TextRole] = "text";
    
    return hash;
}

bool QmlObjectListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.isValid() && role == ObjectRole) {
        m_lsObject.replace(index.row(), value.value<QObject*>());
        emit dataChanged(index, index);
        return true;
    }
    
    return false;
}

QObject *QmlObjectListModel::operator[](int index)
{
    if (index<0 || index>m_lsObject.count())
        return NULL;
    return m_lsObject[index];
}

const QObject* QmlObjectListModel::operator[](int index) const
{
    return m_lsObject[index];
}

bool QmlObjectListModel::contains(QObject* object) const
{
    return m_lsObject.contains(object);
}

int QmlObjectListModel::indexOf(const QObject *object)
{
    int i = 0;
    for (QObject *itr : m_lsObject)
    {
        if (itr == object)
            return i;
        ++i;
    }

    return -1;
}

void QmlObjectListModel::clear(void)
{
    while (rowCount())
    {
        removeAt(0);
    }
}

QObject *QmlObjectListModel::removeAt(int i)
{
    if (i < 0 || i >= m_lsObject.count())
        return NULL;

    beginRemoveRows(QModelIndex(), i, i);
    QObject *removedObject = m_lsObject.takeAt(i);
    endRemoveRows();
    emit countChanged();

    if (MapAbstractItem *item = qobject_cast<MapAbstractItem*>(removedObject))
        item->SetParentModel(NULL);

    return removedObject;
}

QObject *QmlObjectListModel::removeOne(QObject *object)
{
    return removeAt(m_lsObject.indexOf(object));
}

void QmlObjectListModel::removeItmes(const QObjectList &objects)
{
    int count = 0;
    int nBeg = -1;
    QObjectList rvms;
    QList<QObject*>::iterator itr = m_lsObject.begin();
    for (int i = 0; itr != m_lsObject.end(); ++itr, ++i)
    {
        bool bContains = objects.contains(*itr);
        if (objects.contains(*itr))
        {
            if (nBeg < 0)
                nBeg = i;

            if (MapAbstractItem *item = qobject_cast<MapAbstractItem*>(*itr))
                item->SetParentModel(NULL);

            ++count;
        }
        else if (nBeg >= 0)
        {
            beginRemoveRows(QModelIndex(), nBeg, nBeg+count-1);
            m_lsObject.erase(m_lsObject.begin() + nBeg, itr);
            endRemoveRows();
            nBeg = -1;
            i -= count;
            count = 0;
        }
    }

    if (itr == m_lsObject.end() && nBeg >= 0)
    {
        beginRemoveRows(QModelIndex(), nBeg, nBeg+count-1);
        m_lsObject.erase(m_lsObject.begin() + nBeg, itr);
        endRemoveRows();
    }
    emit countChanged();
}

void QmlObjectListModel::insert(int i, QObject *object)
{
    if (!object || m_lsObject.contains(object))
        return;

    if (i < 0 || i > m_lsObject.count())
        i = m_lsObject.count();
    
    QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);

    beginInsertRows(QModelIndex(), i, i);
    m_lsObject.insert(i, object);
    endInsertRows();
    emit countChanged();

    if (MapAbstractItem *item = qobject_cast<MapAbstractItem*>(object))
        item->SetParentModel(this);
}


void QmlObjectListModel::insertList(const QObjectList &objects, int i)
{
	if (objects.size() <= 0)
		return;

	if (i < 0 || i > m_lsObject.count())
		i = m_lsObject.count();

	beginInsertRows(QModelIndex(), i, i + objects.size() -1);
	for (QObject *itr : objects)
	{
		m_lsObject.insert(i++, itr);
        if (MapAbstractItem *item = dynamic_cast<MapAbstractItem *>(itr))
		    item->SetParentModel(this);
	}
    endInsertRows();
    emit countChanged();
}

void QmlObjectListModel::setList(const QList<QObject *> &lstObject)
{
    for(int i = 0; i < lstObject.size(); i++)
    {
        append(lstObject.at(i));
    }
}

void QmlObjectListModel::append(QObject* object)
{
    insert(m_lsObject.count(), object);
}

int QmlObjectListModel::count(void) const
{
    return rowCount();
}

void QmlObjectListModel::deleteListAndContents(void)
{
    for (int i=0; i<m_lsObject.count(); i++) {
        m_lsObject[i]->deleteLater();
    }
    deleteLater();
}

void QmlObjectListModel::clearAndDeleteContents(void)
{
    for (int i=0; i<m_lsObject.count(); i++) {
        m_lsObject[i]->deleteLater();
    }
    clear();
}

void QmlObjectListModel::resetModelBegin()
{
    beginResetModel();
}

void QmlObjectListModel::resetModelEnd()
{
    endResetModel();
}

bool QmlObjectListModel::hasItem(const MapAbstractItem &item) const
{
    QList<QObject*>::const_iterator itr = m_lsObject.begin();
    for (; itr != m_lsObject.end(); ++itr)
    {
        if (*itr == &item || *(MapAbstractItem*)(*itr) == item)
            return true;
    }
    return false;
}

QObject *QmlObjectListModel::lastObject() const
{
    return m_lsObject.count() > 0 ? m_lsObject.last():NULL;
}

bool QmlObjectListModel::operator!=(const QmlObjectListModel &oth) const
{
    return !(oth == *this);
}

bool QmlObjectListModel::operator==(const QmlObjectListModel &oth) const
{
    if (m_lsObject.count() != oth.m_lsObject.count())
        return false;

    QList<QObject*>::const_iterator itr = m_lsObject.begin();
    QList<QObject*>::const_iterator itr2 = oth.m_lsObject.begin();
    for (; itr != m_lsObject.end() && itr2 != oth.m_lsObject.end(); ++itr, ++itr2)
    {
        if (*(MapAbstractItem*)(*itr) != *(MapAbstractItem*)(*itr2))
            return false;
    }

    return true;
}
