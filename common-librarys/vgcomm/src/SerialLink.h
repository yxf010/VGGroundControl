#ifndef SERIALLINK_H
#define SERIALLINK_H

class SerialConfiguration;
class SerialLink;

#include <QSerialPort>
#include <QMetaType>
#include <QLoggingCategory>
#include "LinkInterface.h"
#include "LinkCommand.h"

Q_DECLARE_METATYPE(QSerialPort::SerialPortError)
Q_DECLARE_LOGGING_CATEGORY(SerialLinkLog)

class VGCOMMSHARED_EXPORT SerialCommand : public LinkCommand
{
    Q_OBJECT

    Q_PROPERTY(int      baud            READ baud               WRITE setBaud               NOTIFY baudChanged)
    Q_PROPERTY(int      dataBits        READ dataBits           WRITE setDataBits           NOTIFY dataBitsChanged)
    Q_PROPERTY(int      flowControl     READ flowControl        WRITE setFlowControl        NOTIFY flowControlChanged)
    Q_PROPERTY(int      stopBits        READ stopBits           WRITE setStopBits           NOTIFY stopBitsChanged)
    Q_PROPERTY(int      parity          READ parity             WRITE setParity             NOTIFY parityChanged)
    Q_PROPERTY(QString  portDisplayName READ portDisplayName                                NOTIFY portDisplayNameChanged)
    Q_PROPERTY(bool     usbDirect       READ usbDirect          WRITE setUsbDirect          NOTIFY usbDirectChanged)
public:
    SerialCommand(QObject *p=NULL);

    int  baud() const;
    int  dataBits() const;
    int  flowControl()const;
    int  stopBits()const;
    int  parity()const;
    bool usbDirect()const;

    void setBaud            (int baud);
    void setDataBits        (int databits);
    void setFlowControl     (int flowControl);
    void setStopBits        (int stopBits);
    void setParity          (int parity);
    void setUsbDirect       (bool usbDirect);

    void loadSettings(QSettings& settings, const QString& root);
    void saveSettings(QSettings& settings, const QString& root);
    void updateSettings();
    QString getName()const;
    void setName(const QString& portName);
    QString portDisplayName()const;
    void connectLink();
public:
    static QStringList supportedBaudRates();
    static QString cleanPortDisplayname(const QString name);
protected:
    LinkType type()const;
signals:
    void baudChanged();
    void dataBitsChanged();
    void flowControlChanged();
    void stopBitsChanged();
    void parityChanged();
    void portNameChanged();
    void portDisplayNameChanged();
    void usbDirectChanged(bool usbDirect);
private:
    static void _initBaudRates();
private:
    int m_baud;
    int m_dataBits;
    int m_flowControl;
    int m_stopBits;
    int m_parity;
    QString m_portName;
    QString m_portDisplayName;
    bool m_usbDirect;
};

class SerialLink : public LinkInterface
{
    Q_OBJECT
public:
    // LinkInterface
    void    requestReset();
    bool    isConnected() const;
    qint64  getConnectionSpeed() const;
public slots:
    void writeBytes(const QByteArray &array);
    void linkError(QSerialPort::SerialPortError error);
protected:

private slots:
    void _readBytes(void);
private:
    // Links are only created/destroyed by LinkManager so constructor/destructor is not public
    SerialLink(SerialCommand *config);
    ~SerialLink();
    virtual bool _connect(void);
    virtual void _disconnect(void);
    void _emitLinkError(const QString& errorMsg);
    bool _hardwareConnect(QSerialPort::SerialPortError& error, QString& errorString);
    bool _isBootloader();
    void _resetConfiguration();
signals:
    void aboutToCloseFlag();
private:
    friend class SerialCommand;

    QSerialPort* _port;
    quint64 _bytesRead;
    int     _timeout;
    QMutex  _dataMutex;       // Mutex for reading data from _port
    QMutex  _writeMutex;      // Mutex for accessing the _transmitBuffer.

    volatile bool        _stopp;
    volatile bool        _reqReset;
    QMutex               _stoppMutex;      // Mutex for accessing _stopp
    QByteArray           _transmitBuffer;  // An internal buffer for receiving data from member functions and actually transmitting them via the serial port.
};

#endif // SERIALLINK_H
