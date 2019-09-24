#ifndef __PARAMETERS_LIMIT_H__
#define __PARAMETERS_LIMIT_H__

#include <QObject>
#include <QList>
#include <QMap>
#include <QVariant>

class ParaLimitAbstract;
class QDomElement;

class ParametersLimit : public QObject
{
    Q_OBJECT
    Q_ENUMS(ParamType)
    Q_PROPERTY(QStringList groups READ GetGroups CONSTANT)
public:
    enum ParamType {
        ParamUnknow = QVariant::Invalid,
        ParamBool = QVariant::Bool,
        ParamInt = QVariant::Int,
        ParamUInt = QVariant::UInt,
        ParamFloat = QVariant::Double,
    };
public:
    explicit ParametersLimit(QObject *parent = NULL);
    void LoadParameters(const QDomElement &ele);

    QStringList GetGroups()const;
    QVariant stringToVariant(const QString &id, const QString &val)const;
    QVariant getValue(const QString &id, int index)const;
    int getVarIndex(const QString &id, const QVariant &val)const;

	QString getStrValue(const QString &id, QVariant key);

    Q_INVOKABLE QStringList getParams(const QString &group)const;
    Q_INVOKABLE bool hasParam(const QString &id)const;
    Q_INVOKABLE bool isParamReadonly(const QString &id)const;
protected:
    Q_INVOKABLE ParamType getType(const QString &id)const;
    Q_INVOKABLE QString getName(const QString &id)const;
    Q_INVOKABLE bool isScope(const QString &id)const;
    Q_INVOKABLE QString getLeast(const QString &id)const;
    Q_INVOKABLE QString getBiggest(const QString &id)const;
    Q_INVOKABLE QStringList getDscribes(const QString &id)const;

private:
    void parseParameterLimit(const QDomElement &e);
    ParaLimitAbstract *_getParemeterLimit(const QString &id)const;
private:
    QMap <QString, QList<ParaLimitAbstract*> >  m_params;
};

#endif // __PARAMETER_LIMIT_H__
