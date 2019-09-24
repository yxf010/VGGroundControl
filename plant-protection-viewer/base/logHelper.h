#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <QtCore/QtCore>

class LogHelper : public QObject
{
    Q_OBJECT
private:
    LogHelper();

public:
    static LogHelper *getInstance();

    void   notifyUI(QString strMessage);
private:
    static LogHelper   *m_pInstance;

signals:
    void        sigSendMessage2UI(QString);

};

#endif // LOGHELPER_H
