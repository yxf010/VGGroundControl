#include "VGMainPage.h"
#include <QVariant>
#include "VGApplication.h"
#include <QDateTime>
#include <QSettings>
#if __android__
#include <QtAndroid>
#include <QtAndroidExtras/QAndroidJniEnvironment>
#endif
#include <QQuickItem>
#include <QIcon>
#include <QFontMetrics>

#define TipShowTime 3000
#define DeskTopBGCol "#182750"

VGQmlViewItem::VGQmlViewItem(QObject *item, VGQmlViewItem *parent) : QObject(parent), m_qmlObject(item)
{
    if (item)
        connect(item, &QObject::destroyed, this, &VGQmlViewItem::onQmldestroyed);
}

VGQmlViewItem::~VGQmlViewItem()
{
    VGQmlViewItem *parentItem = dynamic_cast<VGQmlViewItem *>(parent());
    if (parentItem)
        parentItem->m_children.removeAll(this);

    if (m_qmlObject)
        m_qmlObject->deleteLater();

    while (m_children.count())
    {
        delete m_children.takeFirst();
    }
}

void VGQmlViewItem::AddChild(QObject *qmlObj)
{
    if (findQmlObjectItem(qmlObj))
        return;

    if (VGQmlViewItem *tmp = new VGQmlViewItem(qmlObj, this))
        m_children << tmp;
}
QObject *VGQmlViewItem::QmlObject()const
{
    return m_qmlObject;
}

VGQmlViewItem *VGQmlViewItem::FindQmlObjectItem(const QObject *item)
{
    if (item == m_qmlObject)
        return this;

    if (VGQmlViewItem *tmp = findQmlObjectItem(item))
        return tmp;

    foreach(VGQmlViewItem *itr, m_children)
    {
        if (VGQmlViewItem *tmp = itr->FindQmlObjectItem(item))
            return tmp;
    }

    return NULL;
}
QString VGQmlViewItem::GetName()const
{
    return m_qmlObject->objectName();
}

VGQmlViewItem *VGQmlViewItem::GetParent() const
{
    return qobject_cast<VGQmlViewItem*>(parent());
}

void VGQmlViewItem::RemoveItem(QObject *qmlObj)
{
    if (m_qmlObject == qmlObj)
    if (VGQmlViewItem *tmp = (VGQmlViewItem::FindQmlObjectItem(qmlObj)))
    {
        if (VGQmlViewItem *item = tmp->GetParent())
            item->m_children.removeAll(tmp);

        delete tmp;
    }
}

VGQmlViewItem *VGQmlViewItem::findQmlObjectItem(const QObject *item)const
{
    foreach(VGQmlViewItem *itr, m_children)
    {
        if (item == itr->m_qmlObject)
            return itr;
    }
    return NULL;
}

void VGQmlViewItem::onQmldestroyed()
{
    delete this;
}
////////////////////////////////////////////////////////////////////////////////////////////
//VGMainPage
////////////////////////////////////////////////////////////////////////////////////////////
VGMainPage::VGMainPage(QObject *parent) : QObject(parent)
, m_qmlRootWidth(400), m_qmlRootHeight(400)
, m_landscape(false), m_root(NULL)
, m_colBack("#FFFFFF"), m_colBg(DeskTopBGCol)
, m_idTimer(-1), m_bTipVisible(false)
, m_bTipWarn(false), m_bShowTitle(true)
{
    setFont(QFont(QString::fromStdWString(L"微软雅黑"), 12));
    connect(qvgApp, &VGApplication::sigBack, this, &VGMainPage::prcsSigBack);
    _readBtnStyle();
}


VGMainPage::~VGMainPage()
{
}

int VGMainPage::qmlRootWidth()const
{
    return m_qmlRootWidth;
}

void VGMainPage::setQmlRootWidth(int width)
{
    m_qmlRootWidth = width;
    emit qmlRootWidthChanged(m_qmlRootWidth);
}

int VGMainPage::qmlRootHeight()const
{
    return m_qmlRootHeight;
}

void VGMainPage::setQmlRootHeight(int height)
{
    m_qmlRootHeight = height;
    emit qmlRootHeightChanged(m_qmlRootHeight);
}

QObject * VGMainPage::GetCurQmlPage() const
{
    if (m_qmlViewStack.count())
        return m_qmlViewStack.last();

    return NULL;
}

void VGMainPage::SetQmlTip(const QString &tip, bool bWarn)
{
    m_strQmlTip = tip;
    if (tip.isEmpty())
        return;

    emit qmlTipChanged(tip);
    if (m_idTimer > 0)
        killTimer(m_idTimer);

	m_idTimer = startTimer(TipShowTime);
    if (!m_bTipVisible)
    {
        m_bTipVisible = true;
        emit tipVisibleChanged(true);
    }
    if (bWarn != m_bTipWarn)
    {
        m_bTipWarn = bWarn;
        emit tipWarnChanged(bWarn);
    }
}

void VGMainPage::SetMessageBox(const QString & tip)
{
	m_strMsgTip = tip;
	emit messgeTipChanged(tip);
}

bool VGMainPage::GetTitleVisible() const
{
    return m_bShowTitle;
}

void VGMainPage::SetTitleVisible(bool b)
{
    if (m_bShowTitle == b)
        return;
    m_bShowTitle = b;
    emit titleVisibleChanged(b);
}

bool VGMainPage::IsTipVisible() const
{
    return m_bTipVisible;
}

QString VGMainPage::GetQmlTip() const
{
    return m_strQmlTip;
}

QString VGMainPage::GetMessgeTip() const
{
	return m_strMsgTip;
}

bool VGMainPage::IsTipWarn() const
{
    return m_bTipWarn;
}

QFont VGMainPage::font(bool b)
{
    m_font.setBold(b);
    return m_font;
}

QString VGMainPage::getImageProviderUrl(const QString &name, bool enable, bool press)
{
    if (name.isEmpty())
        return QString();

    int stat = QIcon::Normal;
    if (!enable)
        stat = QIcon::Disabled;
    else if (press)
        stat = QIcon::Selected;

    return QString("image://rscLoader/%1|%2").arg(name).arg(stat);
}

qreal VGMainPage::fontHeight(const QFont &f)
{
    QFontMetricsF ms(f);
    return ms.height();
}

qreal VGMainPage::stringWidth(const QFont &f, const QString &str)
{
    QFontMetricsF ms(f);
    return ms.width(str);
}

QFont VGMainPage::littleFont(bool b)
{
    m_fontLittle.setBold(b);
    return m_fontLittle;
}

QFont VGMainPage::miniFont(bool b)
{
    m_miniLittle.setBold(b);
    return m_miniLittle;
}

QFont VGMainPage::biggerFont(bool bBold)
{
    m_fontBig.setBold(bBold);
    return m_fontBig;
}

QFont VGMainPage::largeFont(bool bBold /*= false*/)
{
    m_fontLarge.setBold(bBold);
    return m_fontLarge;
}

void VGMainPage::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == m_idTimer)
    {
        killTimer(m_idTimer);
        m_idTimer = -1;
        m_bTipVisible = false;
        emit tipVisibleChanged(false);
        return;
    }

    QObject::timerEvent(ev);
}

bool VGMainPage::IsShowSrSt() const
{
#ifdef __mobile__
    return false;
#else
    return true;
#endif // __mobile__
}

QColor VGMainPage::GetBackColor() const
{
    return m_colBack;
}

QColor VGMainPage::GetBackGroundColor() const
{
    return m_colBg;
}

void VGMainPage::SetBackColor(const QColor &col)
{
    if (m_colBack == col)
        return;

    m_colBack = col;
    emit backColorChanged(col);
}

void VGMainPage::setFont(const QFont &font)
{
    if (font == m_font)
        return;

    m_font = font;
    m_fontLittle = font;
    m_fontLittle.setPointSize(font.pointSize()-2);
    m_miniLittle = font;
    m_miniLittle.setPointSize(font.pointSize() - 4);
    m_fontBig = m_font;
    m_fontBig.setPointSize(m_font.pointSize() + 2);
    m_fontLarge = m_font;
    m_fontLarge.setPointSize(m_font.pointSize() + 4);
}

void VGMainPage::setLandscape(bool landscape)
{
    m_landscape = landscape;
    emit landscapeChanged(landscape);
}

void VGMainPage::addQmlObject(QObject *qmlObj, QObject *parenQml /*= NULL*/)
{
    if (!qmlObj || (!parenQml && m_root) || (parenQml && !m_root))
        return;

    if (!parenQml)
        m_root = new VGQmlViewItem(qmlObj);
    else if (VGQmlViewItem *item = m_root->FindQmlObjectItem(parenQml))
        item->AddChild(qmlObj);

    connect(qmlObj, SIGNAL(destroyed(QObject *)), this, SIGNAL(destroyedQmlObject(QObject *)));
}

void VGMainPage::setHeaderName(const QString &name)
{
    if (m_headerName == name)
        return;

    m_headerName = name;
    emit headerNameChanged(name);
}

void VGMainPage::SetCurrentQmlPage(QObject *qmlPage)
{
    if (qmlPage && (!m_qmlViewStack.count()||m_qmlViewStack.last()!=qmlPage))
    {
        m_qmlViewStack.push_back(qmlPage);
        emit sigShowPage(qmlPage);
    }
}

bool VGMainPage::isVisible(QObject *qmlPage) const
{
    if (qmlPage && m_qmlViewStack.count())
    {
        QObject *qml = m_qmlViewStack.last();
        if (qml == qmlPage)
            return true;

        QObject *objParent = qml->parent();
        while (objParent)
        {
            if (objParent == qmlPage)
                return true;

            objParent = objParent->parent();
        }
    }

    return false;
}

bool VGMainPage::isCurQmlChild(QObject *qmlPage) const
{
    if (qmlPage && m_qmlViewStack.count())
    {
        QObject *qml = m_qmlViewStack.last();
        if (qml == qmlPage)
            return true;

        QObject *objParent = qmlPage->parent();
        while (objParent)
        {
            if (objParent == qml)
                return true;

            objParent = objParent->parent();
        }
    }

    return false;
}

void VGMainPage::destroyQmlObject(QObject *qmlPage)
{
    bool bPageChange = qmlPage == m_qmlViewStack.last();
    m_qmlViewStack.removeAll(qmlPage);

    if (bPageChange && m_qmlViewStack.count())
        emit sigShowPage(m_qmlViewStack.last());

    if (m_root)
    {
        if (m_root->QmlObject() == qmlPage)
        {
            delete m_root;
            m_root = NULL;
        }
        else
        {
            m_root->RemoveItem(qmlPage);
        }
    }
    qmlPage->deleteLater();
}

void VGMainPage::setScreenHoriz(bool b /*= false*/)
{
#if __android__
	QAndroidJniObject activity = QtAndroid::androidActivity();
	if (b)
        activity.callMethod<void>("setRequestedOrientation", "(I)V", 0);
    else
		activity.callMethod<void>("setRequestedOrientation", "(I)V", 1);

	QAndroidJniEnvironment env;
	if (env->ExceptionCheck())
	{
		qDebug() << "exception occured";
		env->ExceptionClear();
	}
    else
    {
        setLandscape(b);
    }
#endif
}

QString VGMainPage::getTimeString(qint64 tMs, const QString &format)
{
    return QDateTime::fromMSecsSinceEpoch(tMs).toString(format);
}

void VGMainPage::setErrorInfo(const QString &titleName, const QString &errorText)
{
    emit sigErrorInfo(titleName, errorText);
}

void VGMainPage::prcsSigBack()
{
    if (QObject *obj = GetCurQmlPage())
        emit sigBack(obj);
}

bool VGMainPage::IsShowText() const
{
    if (m_btnStyle & Show_Text)
        return true;

    if (m_btnStyle & Show_Imge)
        return false;

    return true;
}

void VGMainPage::SetShowText(bool b)
{
    if (b == IsShowText())
        return;

    if (b)
        m_btnStyle |= Show_Text;
    else
        m_btnStyle &= ~Show_Text;

    emit showTextChanged(IsShowText());
    _writeBtnStyle();
}

bool VGMainPage::IsShowImage() const
{
    return (m_btnStyle & Show_Imge) == Show_Imge;
}

void VGMainPage::SetShowImage(bool b)
{
    if (b == IsShowImage())
        return;

    if (b)
        m_btnStyle |= Show_Imge;
    else
        m_btnStyle &= ~Show_Imge;

    emit showImageChanged(IsShowImage());
    _writeBtnStyle();
}

void VGMainPage::onSigBack()
{
    if (!m_root)
        return;

    if (m_qmlViewStack.count())
    {
        if (!m_root->FindQmlObjectItem(m_qmlViewStack.last()))
            m_qmlViewStack.last()->deleteLater();

        m_qmlViewStack.pop_back();
    }

    if (m_qmlViewStack.count() == 0)
        qvgApp->quit();
    else
        emit sigShowPage(m_qmlViewStack.last());
}

void VGMainPage::_readBtnStyle()
{
    if (QSettings *s = qvgApp->GetSettings())
    {
        s->beginGroup("show_styles");
        m_btnStyle = s->value("btnStyle", (int)Show_ImgeAndText).toInt();
        s->endGroup();
    }
}

void VGMainPage::_writeBtnStyle()
{
    if (QSettings *s = qvgApp->GetSettings())
    {
        s->beginGroup("show_styles");
        s->setValue("btnStyle", m_btnStyle);
        s->endGroup();
    }
}
