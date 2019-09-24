#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "VGApplication.h"
#include <QSettings>
#include <QDir>
#include <QFont>
#include "QLog.h"
#include <QSerialPort>
#include <QUdpSocket>
#include <VGMacro.h>

int main(int argc, char *argv[])
{
	Q_IMPORT_PLUGIN(VGGeoServiceProviderFactory)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

#ifndef __mobile__
    //-- Test for another instance already running. If that's the case, we simply exit.
    QHostAddress host("127.0.0.1");
    QUdpSocket socket;
    if(!socket.bind(host, SINGLE_INSTANCE_PORT, QAbstractSocket::DontShareAddress))
    {
        qWarning() << "Another instance already running. Exiting.";
        exit(-1);
    }
#endif
    VGApplication app(argc, argv);

    //检测是否记录系统运行日志
    bool enableLog = false;
    if (QSettings *settings = app.GetSettings())
    {
        settings->beginGroup("config");
        enableLog = settings->value("enableLog", true).toBool();
        settings->endGroup();
    }
    if (enableLog)
    {
        QString qStLogDir; 
        qStLogDir = app.GetDefualtPath() + QString("vgPlantLog/");
        QDir log_dir(qStLogDir);
        if (!log_dir.exists(qStLogDir))
            log_dir.mkpath(qStLogDir);

         InitSystemLog(qStLogDir);
     }
#ifndef __ios__
    qRegisterMetaType<QSerialPort::SerialPortError>("QSerialPort::SerialPortError");
#endif
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

    app.initApp();
    app.initUI();

    return app.exec();
}
