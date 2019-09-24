#ifndef QLOG_H
#define QLOG_H

#include <QString>
#include <QMutex>
#include <QFile>
#include <QTime>
#include <QDir>
#include <QTextStream>
#include "logHelper.h"
#include <QMessageBox>
#include <QApplication>

static QString g_strMainLogPath;
static int g_nLogDays;
static QDateTime g_clearLastTime;
static QString g_strMessage;
static void InitSystemLog(QString strMainLogPath = "", int nLogDays = 7);

static void g_ClearDir(const QString& tmpdir,const QString& list)
{
    QDir sourceDir(tmpdir);

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach(QFileInfo fileInfo_s, fileInfoList)
    {
        if(fileInfo_s.fileName() == "." || fileInfo_s.fileName() == "..")
            continue;

        if(fileInfo_s.isFile())
        {
            QString strFileName = fileInfo_s.fileName();
            int nFind = list.indexOf(strFileName,0);
            if(nFind < 0)
                QFile(fileInfo_s.filePath()).remove();
        }
    }
}

//保留N天的日志
static void g_ClearDir(int nSaveDays)
{
    QDateTime tt = QDateTime::currentDateTime();
    if(tt.secsTo(g_clearLastTime) < 300)//超过5分钟清理一次
    {
        return;
    }
    g_clearLastTime = tt;

    QString strFileList;
    for(int i = 0; i < nSaveDays; i++)
    {
        QDateTime tt = QDateTime::currentDateTime().addDays(-i);
        QString strLogFile;
        strLogFile = QString("%1.txt,").arg(tt.toString("yyyy-MM-dd"));
        strFileList += strLogFile;
    }

    g_ClearDir(g_strMainLogPath,strFileList);
}

static void g_SystemLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;
    mutex.lock();

    QString strType;
    switch(type)
    {
    case QtDebugMsg:
        strType = QString("[Debug]");
        break;

    case QtWarningMsg:
        strType = QString("[Warn]");
        break;

    case QtCriticalMsg:
    {
        strType = QString("[Critical]");
    }
        break;

    case QtFatalMsg:
        strType = QString("[Fatal]");
        break;
    }

    if (strType == QString("[Critical]"))
    {//CRITICAL: QWindowsBackingStore::flush: BitBlt failed ()，日志有时会出现大量此日志，造成日志文件过大；屏蔽该类型日志
        mutex.unlock();
        return;
    }

    if(strType == QString("[Warn]"))
    {
        //udp中为了实现断线重连，需要不停调用此接口，会引起大量警告日志
        if(msg.contains("QUdpSocket::hasPendingDatagrams()"))
        {
            mutex.unlock();
            return;
        }
    }

    QString curtime = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

    QString message;
    if(type == QtDebugMsg || type == QtWarningMsg)
    {
        QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
        message = QString("%1 %2 %3 %4").arg(strType).arg(curtime).arg(msg).arg(context_info);;
    }
    else
    {
        QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
        message = QString("%1 %2 %3 %4").arg(strType).arg(curtime).arg(msg).arg(context_info);
    }

    LogHelper::getInstance()->notifyUI(message);

    g_ClearDir(g_nLogDays);

    QString strLogFile = g_strMainLogPath;

    strLogFile += QDateTime::currentDateTime().toString("yyyy-MM-dd");
    strLogFile += ".txt";

    QFile file(strLogFile);
    if(file.open(QIODevice::ReadWrite | QFile::Text | QIODevice::Append))
    {
        QTextStream text_stream(&file);
        text_stream << message << "\r\n";
        file.flush();
        file.close();
    }
    else
    {
        QString error;
        error = strLogFile + file.errorString();
        QMessageBox::warning(NULL, QString::fromUtf8("writeLog"), error, QMessageBox::Ok);
    }

    mutex.unlock();
}

static void InitSystemLog(QString strMainLogPath, int nLogDays)
{
    QDir dir;
    if(!strMainLogPath.isEmpty())
    {
        g_strMainLogPath = strMainLogPath;
    }
    else
    {
        QString strLogPath;
        QString curPath = Application::Instance()->GetDefualtPath();
        strLogPath = QString("%1/log/").arg(curPath);
        g_strMainLogPath = strLogPath;
    }

    dir.setPath(g_strMainLogPath);
    if (!dir.exists(g_strMainLogPath))
		dir.mkpath(g_strMainLogPath);

    g_nLogDays = nLogDays;
    g_clearLastTime = QDateTime::currentDateTime();

    qInstallMessageHandler(g_SystemLog);
}

#endif
