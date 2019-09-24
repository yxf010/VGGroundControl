#include "ParametersLimit.h"
#include "ParameterLimit.h"
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>

ParametersLimit::ParametersLimit(QObject *parent) : QObject(parent)
{
}

void ParametersLimit::LoadParameters(const QDomElement &ele)
{
    QDomElement e = ele.firstChildElement("ParameterLimit");
    while (!e.isNull())
    {
        parseParameterLimit(e);
        e = e.nextSiblingElement("ParameterLimit");
    }
}

QStringList ParametersLimit::GetGroups() const
{
    QMap <QString, QList<ParaLimitAbstract*> >::const_iterator itr = m_params.begin();
    QStringList ret;
    for (;itr!=m_params.end();++itr)
    {
        ret << itr.key();
    }
    return ret;
}

QVariant ParametersLimit::stringToVariant(const QString &id, const QString &val) const
{
    QVariant ret;
    if (ParaLimitAbstract *parm = _getParemeterLimit(id))
    {
        ret = parm->ValueFromString(val);
        if (parm->IsScope() && (ret > parm->GetBiggest() || ret < parm->Getleast()))
            return QVariant();
    }
 
    return ret;
}

QStringList ParametersLimit::getParams(const QString &group) const
{
    QString str = group.isEmpty() ? "undef" : group;
    QMap <QString, QList<ParaLimitAbstract*> >::const_iterator itr = m_params.find(str);
    QStringList ret;
    if (itr == m_params.end())
        return ret;

    for (ParaLimitAbstract *item : itr.value())
    {
        ret << item->GetId();
    }
    return ret;
}

ParametersLimit::ParamType ParametersLimit::getType(const QString &id) const
{
    if (ParaLimitAbstract *parm = _getParemeterLimit(id))
        return (ParametersLimit::ParamType)parm->GetType();

    return ParamUnknow;
}

QString ParametersLimit::getName(const QString &id) const
{
    if (ParaLimitAbstract *parm = _getParemeterLimit(id))
        return parm->GetName();

    return QString();
}

bool ParametersLimit::hasParam(const QString &id) const
{
    return _getParemeterLimit(id) != NULL;
}

bool ParametersLimit::isParamReadonly(const QString &id) const
{
    if (ParaLimitAbstract *parm = _getParemeterLimit(id))
        return parm->IsReadOnly();

    return true;
}

bool ParametersLimit::isScope(const QString &id) const
{
    if (ParaLimitAbstract *parm = _getParemeterLimit(id))
        return parm->IsScope();

    return false;
}

QString ParametersLimit::getLeast(const QString &id) const
{
    if (ParaLimitAbstract *parm = _getParemeterLimit(id))
        return parm->ValueToString(parm->Getleast());

    return QString();
}

QString ParametersLimit::getBiggest(const QString &id) const
{
    if (ParaLimitAbstract *parm = _getParemeterLimit(id))
        return parm->ValueToString(parm->GetBiggest());

    return QString();
}

QStringList ParametersLimit::getDscribes(const QString &id) const
{
    if (ParaLimitAbstract *parm = _getParemeterLimit(id))
        return parm->GetDescribes();

    return QStringList();
}

int ParametersLimit::getVarIndex(const QString &id, const QVariant &val) const
{
    if (ParaLimitAbstract *parm = _getParemeterLimit(id))
        return parm->GetPropertyIndex(val);

    return -1;
}

QString ParametersLimit::getStrValue(const QString &id, QVariant val)
{
	if (!val.isValid())
		return QString();
	if (ParaLimitAbstract *parm = _getParemeterLimit(id))	
		return parm->ValueToString(val);

	return QString("%1").arg(val.toUInt()) + QString::fromStdWString(L"原始");
}

QVariant ParametersLimit::getValue(const QString &id, int index) const
{
    if (ParaLimitAbstract *parm = _getParemeterLimit(id))
        return parm->GetValue(index);

    return QVariant();
}

void ParametersLimit::parseParameterLimit(const QDomElement &e)
{
    QString str = e.attribute("type");
    ParaLimitAbstract *param = NULL;

    if ("float" == str)
        param = new ParameterLimit<float>();
    else if ("int" == str)
        param = new ParameterLimit<int>();
    else if ("uint" == str)
        param = new ParameterLimit<uint32_t>();
    else if ("bool" == str)
        param = new ParameterLimit<uint32_t>();

    if (param)
    {
        param->ParseContent(e);
        m_params[e.attribute("group", "undef")].push_back(param);
    }
}

ParaLimitAbstract *ParametersLimit::_getParemeterLimit(const QString &id)const
{
    QMap <QString, QList<ParaLimitAbstract*> >::const_iterator itr = m_params.begin();
    for (; itr != m_params.end(); ++itr)
    {
        for (ParaLimitAbstract *item : itr.value())
        {
            if (item && item->GetId() == id)
                return item;
        }
    }
    return NULL;
}
