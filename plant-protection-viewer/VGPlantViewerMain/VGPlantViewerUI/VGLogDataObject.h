#ifndef VGLOGDATAOBJECT_H
#define VGLOGDATAOBJECT_H

#include <QObject>
#include <QDateTime>
#include <QLocale>
#include <QAbstractListModel>

//日志文件信息结构
class VGLogEntry : public QObject {
    Q_OBJECT
    Q_PROPERTY(uint         id          READ id                             CONSTANT)
    Q_PROPERTY(QDateTime    time        READ time                           NOTIFY timeChanged)
    Q_PROPERTY(uint         size        READ size                           NOTIFY sizeChanged)
    Q_PROPERTY(QString      sizeStr     READ sizeStr                        NOTIFY sizeChanged)
    Q_PROPERTY(bool         received    READ received                       NOTIFY receivedChanged)
    Q_PROPERTY(bool         selected    READ selected   WRITE setSelected   NOTIFY selectedChanged)
    //0 未下载 1 正在下载 2 下载完成
    Q_PROPERTY(int          status      READ status     WRITE setStatus       NOTIFY statusChanged)
    Q_PROPERTY(QString      statusStr   READ statusStr WRITE setStatusStr   NOTIFY statusChanged)

public:
    VGLogEntry();
    VGLogEntry(uint logId, const QDateTime& dateTime = QDateTime(), uint logSize = 0, bool received = false);

    uint        id          () const { return _logID; }
    uint        size        () const { return _logSize; }
    QString     sizeStr     () const;
    QDateTime   time        () const { return _logTimeUTC; }
    bool        received    () const { return _received; }
    bool        selected    () const { return _selected; }
    QString     statusStr   () const { return _statusStr; }
    int         status      () const { return _status; }

    void        setId       (uint id_)          { _logID = id_; }
    void        setSize     (uint size_)        { _logSize = size_;     emit sizeChanged(); }
    void        setTime     (QDateTime date_)   { _logTimeUTC = date_;  emit timeChanged(); }
    void        setReceived (bool rec_)         { _received = rec_;     emit receivedChanged(); }
    void        setSelected (bool sel_)         { _selected = sel_;     emit selectedChanged(); }
    void        setStatusStr(QString stat_)     { _statusStr = stat_;   emit statusChanged(); }
    void        setStatus   (int nStatus)       { _status = nStatus;    emit statusChanged(); }

signals:
    void        idChanged       ();
    void        timeChanged     ();
    void        sizeChanged     ();
    void        receivedChanged ();
    void        selectedChanged ();
    void        statusChanged   ();

private:
    uint        _logID;
    uint        _logSize;
    QDateTime   _logTimeUTC;
    bool        _received;
    bool        _selected;
    QString     _statusStr;
    int         _status;
};

//日志文件model
class VGLogModel : public QAbstractListModel
{
    Q_OBJECT
public:

    enum VGLogModelRoles {
        ObjectRole = Qt::UserRole + 1
    };

    VGLogModel(QObject *parent = 0);

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_INVOKABLE VGLogEntry* get(int index)const;

    int         count           (void) const;
    void        append          (VGLogEntry* entry);
    void        clear           (void);
    VGLogEntry*operator[]      (int i);

    int         rowCount        (const QModelIndex & parent = QModelIndex()) const;
    QVariant    data            (const QModelIndex & index, int role = Qt::DisplayRole) const;

signals:
    void        countChanged    ();

protected:
    QHash<int, QByteArray> roleNames() const;
private:
    QList<VGLogEntry*> _logEntries;
};

class VGLogDataObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName WRITE setFileName)
    Q_PROPERTY(double fileSize READ fileSize WRITE setFileSize)
    //1 表示正在下载 0 表示未下载
    Q_PROPERTY(int downStatus READ downStatus WRITE setDownStatus NOTIFY downStatusChanged)
    Q_PROPERTY(VGLogModel* model READ model  NOTIFY modelChanged)

public:
    explicit VGLogDataObject(QObject *parent = 0);

    Q_INVOKABLE void refresh();         //刷新列表
    Q_INVOKABLE void downLoadLog(int index, int logId);
    Q_INVOKABLE void clearLogModel();  //清空在线日志缓存列表

public:
    QString fileName()const;
	void setFileName(const QString &name);

    double fileSize()const;
    void setFileSize(int size);

    int downStatus()const;
    void setDownStatus(int status);

    VGLogModel *model()const{return m_logModel;}

signals:
    void downStatusChanged(int status);
    void modelChanged();
    void sigUpdateLogList();    //请求更新log列表
    void sigRequestLogData(int logId, long size, long time_utc);  //请求下载日志

public slots:
    //下载文件
    void sltReceiveLogEntry(long time_utc, long size, int id, int num_logs, bool downloaded = false);
    //下载进度
	void sltUpdateLogDownloadStatus(int logId, ulong time_utc, const QString &status);
private:
    QString         m_fileName;
    double          m_fileSize;
    int             m_downStatus;
    VGLogModel      *m_logModel;

};

#endif // VGLOGDATAOBJECT_H
