#ifndef VGMAINPAGE_H
#define VGMAINPAGE_H

#include <QObject>
#include <QList>
#include <QColor>
#include <QFont>

class VGQmlViewItem : public QObject
{
    Q_OBJECT
public:
    VGQmlViewItem(QObject *item, VGQmlViewItem *parent = NULL);
    ~VGQmlViewItem();
    void AddChild(QObject *qmlObj);
    QObject *QmlObject()const;
    VGQmlViewItem *FindQmlObjectItem(const QObject *item);
    QString GetName()const;
    VGQmlViewItem *GetParent()const;
    void RemoveItem(QObject *qmlObj);
private:
    VGQmlViewItem *findQmlObjectItem(const QObject *item)const;
private slots:
    void onQmldestroyed();
private:
    QObject                 *m_qmlObject;
    QList<VGQmlViewItem*>   m_children;
};

class VGMainPage : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int qmlRootWidth READ qmlRootWidth WRITE setQmlRootWidth NOTIFY qmlRootWidthChanged)
    Q_PROPERTY(int qmlRootHeight READ qmlRootHeight WRITE setQmlRootHeight NOTIFY qmlRootHeightChanged)
    Q_PROPERTY(QColor backColor READ GetBackColor WRITE SetBackColor NOTIFY backColorChanged)
    Q_PROPERTY(QColor bgColor READ GetBackGroundColor CONSTANT)
    Q_PROPERTY(QString qmlTip READ GetQmlTip NOTIFY qmlTipChanged)
	Q_PROPERTY(QString messgeTip READ GetMessgeTip NOTIFY messgeTipChanged)
    Q_PROPERTY(bool tipVisible READ IsTipVisible NOTIFY tipVisibleChanged)
    Q_PROPERTY(bool tipWarn READ IsTipWarn NOTIFY tipWarnChanged)
    //是否横屏
    Q_PROPERTY(bool landscape READ landscape WRITE setLandscape NOTIFY landscapeChanged)
    Q_PROPERTY(bool titleVisible READ GetTitleVisible WRITE SetTitleVisible NOTIFY titleVisibleChanged)
    Q_PROPERTY(QString headerName READ headerName WRITE setHeaderName NOTIFY headerNameChanged)
    Q_PROPERTY(QObject* curQmlPage READ GetCurQmlPage WRITE SetCurrentQmlPage NOTIFY sigShowPage)
    Q_PROPERTY(bool showText READ IsShowText WRITE SetShowText NOTIFY showTextChanged)
    Q_PROPERTY(bool showImage READ IsShowImage WRITE SetShowImage NOTIFY showImageChanged)
    Q_PROPERTY(bool showSrSt READ IsShowSrSt CONSTANT)
public:
    enum ButtonShowStayle
    {
        Show_Imge = 1,
        Show_Text = 2,
        Show_ImgeAndText = Show_Imge|Show_Text,
    };
public:
    explicit VGMainPage(QObject *parent = 0);
    ~VGMainPage();

    Q_INVOKABLE void setScreenHoriz(bool b = false);
    Q_INVOKABLE QString getTimeString(qint64 tMs, const QString &format = "yyyy-MM-dd hh:mm:ss");
    Q_INVOKABLE QFont   font(bool bBold = false);
    Q_INVOKABLE QString getImageProviderUrl(const QString &name, bool enable = true, bool press = false);
    Q_INVOKABLE qreal fontHeight(const QFont &f);
    Q_INVOKABLE qreal stringWidth(const QFont &f, const QString &str);
public:
    int qmlRootWidth()const;
    void setQmlRootWidth(int width);

    int qmlRootHeight()const;
    void setQmlRootHeight(int height);

    void setFont(const QFont &font);

    bool landscape()const { return m_landscape;}
    void setLandscape(bool landscape);

    QString headerName()const { return m_headerName; }
    void setHeaderName(const QString &name);

    void SetCurrentQmlPage(QObject *qmlPage);
    QObject *GetCurQmlPage()const;

    void SetQmlTip(const QString &tip, bool bWarn);
	void SetMessageBox(const QString &tip);
    QColor  GetBackGroundColor()const;
    bool IsShowSrSt()const;
protected:
    bool    IsShowText()const;
    void    SetShowText(bool b);
    bool    IsShowImage()const;
    void    SetShowImage(bool b);
    QColor  GetBackColor()const;
    void    SetBackColor(const QColor &);
    bool    GetTitleVisible()const;
    void    SetTitleVisible(bool b);
    bool    IsTipVisible()const;
    QString GetQmlTip()const;
	QString GetMessgeTip() const;
    bool    IsTipWarn()const;

    Q_INVOKABLE QFont   miniFont(bool bBold = false);
    Q_INVOKABLE QFont   littleFont(bool bBold = false);
    Q_INVOKABLE QFont   biggerFont(bool bBold = false);
    Q_INVOKABLE QFont   largeFont(bool bBold = false);
    Q_INVOKABLE void    onSigBack();
    Q_INVOKABLE bool    isVisible(QObject *qmlPage)const;
    Q_INVOKABLE bool    isCurQmlChild(QObject *qmlPage)const;
    Q_INVOKABLE void    addQmlObject(QObject *qmlPage, QObject *parenPage = NULL);
    Q_INVOKABLE void    destroyQmlObject(QObject *qmlPage);
    //overload
    void timerEvent(QTimerEvent *ev);
signals:
    void    qmlRootWidthChanged(int qmlwidth);
    void    qmlRootHeightChanged(int qmlheight);
    void    landscapeChanged(bool landscape);
    void    sigBack(QObject *page);
    void    headerNameChanged(const QString &name);
	void    sigErrorInfo(const QString &titleName, const QString &errorText); //通知qml显示错误
    void    sigShowPage(QObject *page);
    void    destroyedQmlObject(QObject *page);
    void    titleVisibleChanged(bool b);
    void    backColorChanged(const QColor &bg);
    void    showTextChanged(bool b);
    void    showImageChanged(bool b); 
    void    qmlTipChanged(const QString &tip);
	void	messgeTipChanged(const QString &tip);
    void    tipVisibleChanged(bool visible);
    void    tipWarnChanged(bool visible);
public slots:
    void    setErrorInfo(const QString &titleName, const QString &errorText);
    void    prcsSigBack();
private:
    void _readBtnStyle();
    void _writeBtnStyle();
private:
	int         m_qmlRootWidth;
    int         m_qmlRootHeight;
    bool        m_landscape;
    bool        m_bTipVisible;
    bool        m_bTipWarn;
    int                 m_btnStyle;
    int                 m_idTimer;
    bool                m_bShowTitle;
    QString     m_headerName;
    QString     m_strQmlTip;
	QString		m_strMsgTip;

    QColor      m_colBack;
    QColor      m_colBg;
    QFont       m_font;
    QFont       m_miniLittle;
    QFont       m_fontLittle;
    QFont       m_fontBig;
    QFont       m_fontLarge;
    VGQmlViewItem       *m_root;
    QList<QObject*>     m_qmlViewStack;
};

#endif // VGMAINPAGE_H
