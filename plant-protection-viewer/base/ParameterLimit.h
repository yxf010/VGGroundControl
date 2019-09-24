#ifndef __PARAMETER_LIMIT_H__
#define __PARAMETER_LIMIT_H__

#include <QVariant>
#include <QString>
#include <QPair>
#include <QList>
#include <QMetaType>

class QDomElement;
class ParaLimitAbstract {
public:
    enum ParamStat{
        Normal,
        Base16,
        IPType,
        Readonly = 1<<4,
        Stict = Readonly<<1,
        ListVal = Stict<<1,
    };
    typedef QPair<QVariant, QString> ValuePair;
public:
    ParaLimitAbstract();
    virtual~ParaLimitAbstract() {}

    QString GetId()const;
    void SetId(const QString &id);
	QString GetUIntType()const;
	void SetUIntType(const QString &uintType);
    QString GetName()const;
    void SetName(const QString &str);
    void SetScope(bool b);
    bool IsScope()const;
    void SetReadOnly(bool b);
    bool IsReadOnly()const;
    void SetStrict(bool b);
    bool IsStrict()const;
    int GetShowType()const;
    void AddValue(const QVariant &val, const QString &describe = QString());

    QStringList GetDescribes()const;
    bool IsInScope(const QVariant &val)const;
    QVariant GetValue(int idx)const;
    virtual int GetType()const = 0;
    virtual int GetPropertyIndex(const QVariant &val)const = 0;
    void ParseContent(const QDomElement &e);
    void SetBiggest(const QVariant &val);
    QVariant GetBiggest()const;
    void SetLeast(const QVariant &v);
    QVariant Getleast()const;
public:
    QString ValueToString(const QVariant &val);
    QVariant ValueFromString(const QString &str);
protected:
    void setShowType(int tp);
    void parseShowType(const QString &s);
    int GetIndexOf(const QVariant &val)const;
protected:
    uint32_t                m_stat;
    QString                 m_id;
    QString                 m_name;
    QList<ValuePair>        m_vCanValues;
};

template<class T>
class ParameterLimit : public ParaLimitAbstract {
public:
    ParameterLimit() : ParaLimitAbstract(){ }
    int GetType()const
    {
        return qMetaTypeId<T>();
    }
    int GetPropertyIndex(const QVariant &val)const
    {
        if (IsScope() || m_vCanValues.size() < 1 || val.type() != GetType())
            return -1;

        if (!IsStrict())
            return GetIndexOf(val);

        QList<ValuePair>::const_iterator itr = m_vCanValues.begin();
        T v = val.value<T>();
        T last = itr->first.value<T>();
        if (v <= last)
            return 0;
        itr++;
        for (int i = 1; itr != m_vCanValues.end(); ++i, ++itr)
        {
            T tmp = itr->first.value<T>();
            if (v <= (tmp + last) / 2)
                return i - 1;
            last = tmp;
        }

        return m_vCanValues.size() - 1;
    }
};

#endif // __PARAMETER_LIMIT_H__
