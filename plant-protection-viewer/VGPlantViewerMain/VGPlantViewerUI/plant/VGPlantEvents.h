#ifndef __VG_PLANTEVENTS_H__
#define __VG_PLANTEVENTS_H__

#include <QObject>
#include <QColor>
#include "QmlObjectListModel.h"

class VGPlantEventDescribe;
class QDomElement;

enum Event_Leave {
    Event_Info,
    Event_SpeakInfo,
    Event_Fatal,

    Event_End,
};

class VGPlantEvent : public QObject
{
    Q_OBJECT
    Q_ENUMS(Event_Leave)
    Q_PROPERTY(QString evtStr READ GetEvent CONSTANT)
    Q_PROPERTY(QColor color READ GetEventColor CONSTANT)
    Q_PROPERTY(bool visible READ IsVisible NOTIFY visibleChanged)
public:
    VGPlantEvent(const QString &id, VGPlantEventDescribe *evt);
    QString GetDescribe()const;
    Event_Leave GetLeave()const;
    void SetVisible(bool b);
public:
    static QString GetEventString(Event_Leave);
protected:
    QColor GetEventColor()const;
    QString GetEvent()const;
    bool IsVisible()const;
    Q_INVOKABLE void remove();
signals:
    void removed(VGPlantEvent *e);
    void visibleChanged();
private:
    qint64                  m_time;
    QString                 m_strUav;
    VGPlantEventDescribe    *m_event;
    bool                    m_bVisible;
};

class VGPlantEventDescribe
{
public:
    QString GetTip()const;
    Event_Leave GetLeave()const;
    int GetEventID()const;
public:
    static VGPlantEventDescribe *Parse(const QDomElement &e);
protected:
    VGPlantEventDescribe();
private:
    static Event_Leave transLeave(const QString &lv);
private:
    int         m_evtID;
    Event_Leave m_leave;
    QString     m_tip;
};

class VGPlantEvents :public QObject
{
    Q_OBJECT
    Q_PROPERTY(QmlObjectListModel *events READ GetEvents CONSTANT)
    Q_PROPERTY(bool showAll READ IsShowAll WRITE SetShowAll NOTIFY showAllChanged)
public:
    VGPlantEvents(QObject *p=NULL);
    ~VGPlantEvents();

    void Parse(const QDomElement &e);
    void AddEvent(const QString &id, int evt);
    QmlObjectListModel *GetEvents();
protected:
    bool IsShowAll()const;
    void SetShowAll(bool b);
protected slots:
    void onEventRemoved(VGPlantEvent *e);
signals:
    void showAllChanged();
private:
    void _saveConfig();
    void _loadConfig();
private:
    bool                                m_bShowAll;
    QmlObjectListModel                  *m_events;
    QMap<int, VGPlantEventDescribe*>    m_eventDscrs;
};

#endif // __VG_PLANTEVENTS_H__
