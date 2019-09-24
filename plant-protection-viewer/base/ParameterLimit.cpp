#include "ParameterLimit.h"
#include <QDomElement>
#include <ParametersLimit.h>

static uint ip2UInt(QString ip)
{
	QStringList iplist = ip.split(".");
    if (iplist.size() != 4)
        return 0;

    uint ret = 0;
    for (int i = 0; i < 4; ++i)
    {
        int tmp = iplist.at(3-i).toUInt();
        if (tmp < 0 || tmp>255)
            return 0;
        ((uint8_t*)&ret)[i] = tmp;
    }
	return ret;
}

static QString uint2Ip(uint ip)
{
    uint8_t *tmp = (uint8_t *)&ip;
	return QString("%1.%2.%3.%4").arg(unsigned(tmp[3])).arg(unsigned(tmp[2])).arg(unsigned(tmp[1])).arg(unsigned(tmp[0]));
}

static QString intVarientToStr(const QVariant &val, int tp)
{
    if (val.type() != QVariant::Int && val.type() != QVariant::UInt)
        return QString();

    QString ret;
    if (tp == ParaLimitAbstract::Normal)
        ret =  val.type() == QVariant::Int ? QString::number(val.toInt()) : QString::number(val.toUInt());
    else if (tp == ParaLimitAbstract::Base16)
        ret = val.type() == QVariant::Int ? QString("%1").arg(val.toInt(), 8, 16, QChar('0')) : QString("%1").arg(val.toUInt(), 8, 16, QChar('0'));
    else if (tp == ParaLimitAbstract::IPType)
        ret = uint2Ip(val.toUInt());

    return ret;
}

static QVariant intVarientFromStr(const QString &val, int tp, bool bUint=false)
{
    QVariant ret;
    if (tp == ParaLimitAbstract::Normal)
        bUint ? ret.setValue<uint>(val.toUInt()) : ret.setValue<int>(val.toInt());
    else if (tp == ParaLimitAbstract::Base16)
        bUint ? ret.setValue<uint>(val.toUInt(NULL, 16)) : ret.setValue<int>(val.toInt(NULL, 16));
    else if (tp == ParaLimitAbstract::IPType)
        bUint ? ret.setValue<uint>(ip2UInt(val)) : ret.setValue<int>(ip2UInt(val));

    return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////
//ParaLimitAbstract
/////////////////////////////////////////////////////////////////////////////////////////////
ParaLimitAbstract::ParaLimitAbstract():m_stat(Normal)
{
}

QString ParaLimitAbstract::GetId() const
{
    return m_id;
}

void ParaLimitAbstract::SetId(const QString &id)
{
    m_id = id;
}

QString ParaLimitAbstract::GetName() const
{
    return m_name;
}

void ParaLimitAbstract::SetName(const QString &str)
{
    m_name = str;
}

void ParaLimitAbstract::SetScope(bool b)
{
    if (b)
        m_stat &= ~ListVal;
    else
        m_stat |= ListVal;
}

bool ParaLimitAbstract::IsScope() const
{
    return 0==(m_stat & ListVal);
}

void ParaLimitAbstract::SetReadOnly(bool b)
{
    if (!b)
        m_stat &= ~Readonly;
    else
        m_stat |= Readonly;
}

bool ParaLimitAbstract::IsReadOnly() const
{
    return m_stat & Readonly;
}

void ParaLimitAbstract::SetStrict(bool b)
{
    if (!b)
        m_stat &= ~Stict;
    else
        m_stat |= Stict;
}

bool ParaLimitAbstract::IsStrict() const
{
    return m_stat & Stict;
}

void ParaLimitAbstract::setShowType(int tp)
{
    int tmp = GetType();
    if ((tmp == QMetaType::Int || tmp == QMetaType::UInt) && tp<= IPType)
    {
        m_stat &= ~0x0f;
        m_stat |= tp;
    }
}

void ParaLimitAbstract::parseShowType(const QString &s)
{
    if (s == "base16")
        setShowType(Base16);
    else if (s == "ip")
        setShowType(IPType);
    else
        setShowType(Normal);
}

int ParaLimitAbstract::GetShowType() const
{
    int tp = GetType();
    if (tp == QMetaType::Int || tp == QMetaType::UInt)
        return m_stat & 0x0f;

    return 0;
}

void ParaLimitAbstract::AddValue(const QVariant &val, const QString &describe /*= QString()*/)
{
    if (IsScope() || val.type()!= GetType())
        return;

    if (!IsStrict() || (IsStrict() && (m_vCanValues.size() <= 0 || m_vCanValues.last().first < val)))
        m_vCanValues << ValuePair(val, describe);
}

void ParaLimitAbstract::SetBiggest(const QVariant &val)
{
    if (IsScope())
    {
        if (m_vCanValues.size() > 1)
            m_vCanValues.removeLast();
        m_vCanValues << ValuePair(val, QString());
    }
}

QVariant ParaLimitAbstract::GetBiggest() const
{
    if (IsScope() && m_vCanValues.size())
        return m_vCanValues.last().first;

    return QVariant();
}

void ParaLimitAbstract::SetLeast(const QVariant &v)
{
    if (IsScope())
    {
        if (m_vCanValues.size() > 1)
            m_vCanValues.removeFirst();

        m_vCanValues.push_front(ValuePair(v, QString()));
    }
}

QVariant ParaLimitAbstract::Getleast() const
{
    if (IsScope() && m_vCanValues.size())
        return m_vCanValues.first().first;

    return QVariant();
}

QStringList ParaLimitAbstract::GetDescribes() const
{
    QStringList ret;
    if (IsScope())
        return ret;

    for (const ValuePair &itr : m_vCanValues)
    {
        ret << itr.second;
    }
    return ret;
}

bool ParaLimitAbstract::IsInScope(const QVariant &val) const
{
    if (val.type() == GetType())
        return val >= Getleast() && val <= GetBiggest();

    return false;
}

QVariant ParaLimitAbstract::GetValue(int idx) const
{
    if (IsScope() || idx < 0 || idx >= m_vCanValues.count())
        return QVariant();

    return m_vCanValues.at(idx).first;
}

void ParaLimitAbstract::ParseContent(const QDomElement &e)
{
    SetId(e.attribute("id"));
    SetName(e.attribute("name"));
    QString ls = e.attribute("list");
    SetReadOnly(e.attribute("readonly", "false") == "true");
    SetStrict(e.attribute("strict", "false") == "false");
    parseShowType(e.attribute("show", "normal"));
    bool bScope = ls.isEmpty() && e.hasAttribute("biggest") && e.hasAttribute("least");
    SetScope(bScope);
    if (bScope)
    {
        QVariant tmp = ValueFromString(e.attribute("biggest"));
        SetBiggest(tmp);
        tmp = ValueFromString(e.attribute("least"));
        SetLeast(tmp);
    }
    else
    {
        for (const QString &itr : ls.split(";", QString::SkipEmptyParts))
        {
            QStringList ls = itr.split(":", QString::SkipEmptyParts);
            QVariant tmp = ValueFromString(ls.first());
            AddValue(tmp, ls.size() > 1 ? ls.at(1) : QString());
        }
    }
}

QString ParaLimitAbstract::ValueToString(const QVariant &val)
{
    int idx = GetIndexOf(val);
    if (idx >= 0)
    {
        QString dcr = m_vCanValues.at(idx).second;
        if (!dcr.isEmpty())
            return dcr;
    }

    switch (val.type())
    {
    case QVariant::Bool:
        return val.toBool() ? QString::fromStdWString(L"是") : QString::fromStdWString(L"否");
    case QVariant::Int:
    case QVariant::UInt:
        return intVarientToStr(val, GetShowType());
    case QVariant::Double:
	case QMetaType::Float:
        return QString::number(val.toReal());
    default:
        break;
    }
    return QString();
}

QVariant ParaLimitAbstract::ValueFromString(const QString &str)
{
    bool b = true;
    QVariant ret;
	switch (GetType())
	{
	case QMetaType::Bool:
		ret = str.compare("true") ? false : true;
		break;
	case QMetaType::Int:
    case QMetaType::UInt:
        ret = intVarientFromStr(str, GetShowType(), QMetaType::UInt == GetType());
        break;
    case QMetaType::Double:
        ret = str.toDouble(&b);
        break;
    case QMetaType::Float:
        ret.setValue<float>(str.toFloat(&b));
        break;
    default:
        b = false;
        break;
    }

    return b ? ret : QVariant();
}

int ParaLimitAbstract::GetIndexOf(const QVariant &val)const
{
    QList<ValuePair>::const_iterator itr = m_vCanValues.begin();
    for (int i = 0; itr!=m_vCanValues.end(); ++i,++itr)
    {
        if (itr->first == val)
            return i;
    }
    return -1;
}
