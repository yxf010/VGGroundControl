#ifndef _LINKMANAGER_H_
#define _LINKMANAGER_H_

#include <QList>
#include <QMap>
#include <QTimer>
#include "vgcomm_global.h"
#include "LinkCommand.h"

class MAVLinkProtocol;
class SerialCommand;

class VGCOMMSHARED_EXPORT LinkManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool           isBluetoothAvailable READ isBluetoothAvailable  CONSTANT)
    Q_PROPERTY(bool           openUdp              READ IsOpenUdp             WRITE SetOpenUdp  NOTIFY openUdpChanged)
    Q_PROPERTY(QStringList    serialBaudRates      READ serialBaudRates       CONSTANT)
    Q_PROPERTY(QStringList    serialPortStrings    READ serialPortStrings     NOTIFY commPortStringsChanged)
    Q_PROPERTY(QStringList    serialPorts          READ serialPorts           NOTIFY commPortsChanged)
    /// Unit Test has access to private constructor/destructor
public:
    explicit LinkManager(QObject *parent = 0);
    ~LinkManager();
    bool isBluetoothAvailable       (void);
    QStringList         linkTypeStrings     (void) const;
    QStringList         serialBaudRates     (void);
    QStringList         serialPortStrings   (void);
    QStringList         serialPorts         (void);

    bool containsLink(LinkInterface *link)const;
    void setAutoconnectPixhawk  (bool autoconnect);
    void setAutoconnect3DRRadio (bool autoconnect);
    void setAutoconnectPX4Flow  (bool autoconnect);
    void setAutoconnectRTKGPS     (bool autoconnect);
    void setAutoconnectLibrePilot (bool autoconnect);

    void loadLinkCommandList();
    void saveLinkCommandList();
    /// @return true: specified link is an autoconnect link
    bool isAutoconnectLink(LinkCommand *cmd)const;
    // Override from QGCTool
    //virtual void setToolbox(QGCToolbox *toolbox);
    MAVLinkProtocol *getMavlinkProtocol()const;
    void setConnectUAVStatus(bool isConnect);
    void setUAVRadioBaud(long baud); //set baud rate for UAV radio station
    bool IsOpenUdp()const;
    void SetOpenUdp(bool b);
    QList<LinkCommand *> linkCmds()const;
    void DeleteLink(LinkInterface *cmd);
protected:
    Q_INVOKABLE LinkCommand *createLinkCmd(int tp);
    Q_INVOKABLE void endCreateLinkCmd(LinkCommand* cmd, bool bLink=true);
    Q_INVOKABLE bool isLinked(const QString &name, int tp);
    Q_INVOKABLE void shutdown(int tp);

    QList<LinkCommand *> _getLinkCmds(LinkCommand::LinkType tp);
signals:
    void autoconnectPixhawkChanged(bool autoconnect);
    void autoconnect3DRRadioChanged(bool autoconnect);
    void autoconnectPX4FlowChanged(bool autoconnect);
    void autoconnectRTKGPSChanged(bool autoconnect);
    void autoconnectLibrePilotChanged(bool autoconnect);
    void openUdpChanged(bool b);
    void linkDeleted(LinkInterface* link);
    void linkInactive(LinkInterface* link);
    void linkConnected(LinkInterface* link);
    void commPortStringsChanged();
    void commPortsChanged();
    void communicationError(const QString& title, const QString& error);
    void _shutdown(const QList<LinkCommand*> &cmds);
private slots:
    void _linkConnect(bool b);
    void onShutdown(const QList<LinkCommand*> &cmds);
    void onCommandDeleted(LinkInterface *link);
#ifndef __ios__
    void _activeLinkCheck(void);
#endif
private:
    bool _connectionsSuspendedMsg(void);
    void _updateAutoConnectLinks(void);
    void _updateSerialPorts();
	bool _setAutoconnectWorker(bool& currentAutoconnect, bool newAutoconnect, const char* autoconnectKey);	
    void _addLink(LinkInterface *link);
    bool _closeAllBlt(void);
    void _checkUdpLink();
    bool _readUdpLinkConfig()const;
    void _writeUdpLinkConfig()const;
    void _updateAutoSerialLinks(QStringList &currentPorts);
#ifndef __ios__
    SerialCommand* _autoconnectCommandOfName(const QString& portName);
#endif
    bool    _configurationsLoaded;                      ///< true: Link configurations have been loaded
    QTimer  _portListTimer;
    uint32_t _mavlinkChannelsUsedBitMask;

    MAVLinkProtocol     *m_mavlinkProtocol;

    QList<LinkCommand *> m_linkCmds;
    QList<SerialCommand *> m_autolinkCmds;
    QMap<QString, int>  _autoconnectWaitList;   ///< key: SerialPortInfo.systemLocation, value: wait count
    QStringList m_commPortList;
    QStringList m_commPortDisplayList;

    bool _autoconnectPixhawk;
    bool _autoconnect3DRRadio;
    bool _autoconnectPX4Flow;
    bool _autoconnectRTKGPS;
    bool _autoconnectLibrePilot;
    bool _isConnectUAV;
    bool m_bOpenUdp;

    long _UAVRadioBaud;         //无线电波特率
#ifndef __ios__
    QTimer                  _activeLinkCheckTimer;                  ///< Timer which checks for a vehicle showing up on a usb direct link
    QList<SerialCommand*>   m_activeLinkCheckList;                   ///< List of links we are waiting for a vehicle to show up on
#endif
    static const char*  _settingsGroup;
    static const char*  _autoconnectPixhawkKey;
    static const char*  _autoconnect3DRRadioKey;
    static const char*  _autoconnectPX4FlowKey;
    static const char*  _autoconnectRTKGPSKey;
    static const char*  _autoconnectLibrePilotKey;
};

#endif
