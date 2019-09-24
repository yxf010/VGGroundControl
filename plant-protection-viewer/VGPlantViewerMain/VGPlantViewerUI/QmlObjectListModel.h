#ifndef QmlObjectListModel_H
#define QmlObjectListModel_H

#include <QAbstractListModel>

class MapAbstractItem;

class QmlObjectListModel : public QAbstractListModel
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    QmlObjectListModel(QObject* parent = NULL);
    QmlObjectListModel(QmlObjectListModel &model);
    ~QmlObjectListModel();  
    /// Returns true if any of the items in the list are dirty. Requires each object to have
    /// a dirty property and dirtyChanged signal.
    Q_INVOKABLE QObject* get(int index)const;

    // Property accessors
    
    int count(void) const;

    void setList(const QList<QObject*> &lstObject);
    void append(QObject* object);
    void clear(void);
    QObject* removeAt(int i);
    QObject* removeOne(QObject* object);
	void removeItmes(const QObjectList &objects);
    void insert(int i, QObject* object);
	void insertList(const QObjectList &objects, int i=-1);
    QObject* operator[](int i);
    const QObject *operator[](int i) const;
    bool contains(QObject* object)const;
    int indexOf(const QObject *object);
    template<class T>
    T value(int index)const
    {
        return qobject_cast<T>(m_lsObject.at(index));
    }
    /// Calls deleteLater on all items and this itself.
    void deleteListAndContents(void);

    /// Clears the list and calls delete on each entry
    void clearAndDeleteContents(void);

    void resetModelBegin();
    void resetModelEnd();

    bool operator==(const QmlObjectListModel &oth)const;
    bool operator!=(const QmlObjectListModel &oth)const;
    bool hasItem(const MapAbstractItem &item)const;
    QObject *lastObject()const;
signals:
    void countChanged();
private:
    // Overrides from QAbstractListModel
    virtual int	rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QHash<int, QByteArray> roleNames(void) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
private:
    QList<QObject*> m_lsObject;
};

#endif
