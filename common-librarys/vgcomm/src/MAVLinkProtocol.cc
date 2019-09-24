/*===================================================================
======================================================================*/

/**
 * @file
 *   @brief Implementation of class MAVLinkProtocol
 *   @author Lorenz Meier <mail@qgroundcontrol.org>
 */

#include <iostream>

#include <QDebug>
#include <QTime>
#include <QSettings>
#include <QStandardPaths>
#include <QtEndian>
#include <QMetaType>
#include <QDir>
#include <QDateTime>

#include "MAVLinkProtocol.h"
#include "LinkManager.h"
#include "base.h"

Q_DECLARE_METATYPE(mavlink_message_t)

/**
 * The default constructor will create a new MAVLink object sending heartbeats at
 * the MAVLINK_HEARTBEAT_DEFAULT_RATE to all connected links.
 */
MAVLinkProtocol::MAVLinkProtocol(LinkManager* linkMgr)
    :m_multiplexingEnabled(false)
    , m_authEnabled(false)
    , m_enable_version_check(true)
    , m_paramRetransmissionTimeout(350)
    , m_paramRewriteTimeout(500)
    , m_paramGuardEnabled(true)
    , m_actionGuardEnabled(false)
    , m_actionRetransmissionTimeout(100)
    , versionMismatchIgnore(false)
    , systemId(BASE::defaultSystemId)
    , componentId(BASE::defaultComponentId)
#ifndef __mobile__
    , _logSuspendError(false)
    , _logSuspendReplay(false)
    , _logPromptForSave(false)
    ,_LogFile(NULL)
#endif
    , _linkMgr(linkMgr)
{
    QString qStMavLogDir = Application::Instance()->GetDefualtPath() + QString("log/mavlink");
    QDir log_dir(qStMavLogDir);
    if (!log_dir.exists(qStMavLogDir))
    {
        log_dir.mkpath(qStMavLogDir);
    }
#ifndef __mobile__
    _filename = QDir(qStMavLogDir).filePath("vgmav_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".mavlink");
#endif
    qRegisterMetaType<mavlink_message_t>("mavlink_message_t");

    loadSettings();
}

MAVLinkProtocol::~MAVLinkProtocol()
{
    storeSettings();
    
#ifndef __mobile__
    _stopLogging();
#endif
}

void MAVLinkProtocol::loadSettings()
{
    // Load defaults from settings
    QSettings settings;
    settings.beginGroup("QGC_MAVLINK_PROTOCOL");
    enableVersionCheck(settings.value("VERSION_CHECK_ENABLED", m_enable_version_check).toBool());
    enableMultiplexing(settings.value("MULTIPLEXING_ENABLED", m_multiplexingEnabled).toBool());

    // Only set system id if it was valid
    int temp = settings.value("GCS_SYSTEM_ID", systemId).toInt();
    if (temp > 0 && temp < 256)
    {
        systemId = temp;
    }

    temp = settings.value("GCS_COMPONENT_ID", componentId).toInt();
    if (temp >= 0 && temp < 256)
    {
        componentId = temp;
    }

    // Set auth key
    m_authKey = settings.value("GCS_AUTH_KEY", m_authKey).toString();
    enableAuth(settings.value("GCS_AUTH_ENABLED", m_authEnabled).toBool());

    // Parameter interface settings
    bool ok;
    temp = settings.value("PARAMETER_RETRANSMISSION_TIMEOUT", m_paramRetransmissionTimeout).toInt(&ok);
    if (ok) m_paramRetransmissionTimeout = temp;
    temp = settings.value("PARAMETER_REWRITE_TIMEOUT", m_paramRewriteTimeout).toInt(&ok);
    if (ok) m_paramRewriteTimeout = temp;
    m_paramGuardEnabled = settings.value("PARAMETER_TRANSMISSION_GUARD_ENABLED", m_paramGuardEnabled).toBool();
    settings.endGroup();
}

void MAVLinkProtocol::storeSettings()
{
    // Store settings
    qDebug() << "Store GCS_SYSTEM_ID = " << systemId << "GCS_COMPONENT_ID" << componentId;
    QSettings settings;
    settings.beginGroup("QGC_MAVLINK_PROTOCOL");
    settings.setValue("VERSION_CHECK_ENABLED", m_enable_version_check);
    settings.setValue("MULTIPLEXING_ENABLED", m_multiplexingEnabled);
    settings.setValue("GCS_SYSTEM_ID", systemId);
    settings.setValue("GCS_COMPONENT_ID", componentId);
    settings.setValue("GCS_AUTH_KEY", m_authKey);
    settings.setValue("GCS_AUTH_ENABLED", m_authEnabled);
    // Parameter interface settings
    settings.setValue("PARAMETER_RETRANSMISSION_TIMEOUT", m_paramRetransmissionTimeout);
    settings.setValue("PARAMETER_REWRITE_TIMEOUT", m_paramRewriteTimeout);
    settings.setValue("PARAMETER_TRANSMISSION_GUARD_ENABLED", m_paramGuardEnabled);

    settings.endGroup();
}

void MAVLinkProtocol::resetMetadataForLink(const LinkInterface *link)
{
    int channel = link->getMavlinkChannel();
    totalReceiveCounter[channel] = 0;
    totalLossCounter[channel] = 0;
    totalErrorCounter[channel] = 0;
    currReceiveCounter[channel] = 0;
    currLossCounter[channel] = 0;
}

/**
 * This method parses all incoming bytes and constructs a MAVLink packet.
 * It can handle multiple links in parallel, as each link has it's own buffer/
 * parsing state machine.
 * @param link The interface to read from
 * @see LinkInterface
 **/
void MAVLinkProtocol::receiveBytes(LinkInterface* link, QByteArray b)
{
    if (!_linkMgr->containsLink(link))
        return;

    mavlink_message_t message;
    mavlink_status_t status;

    int mavlinkChannel = link->getMavlinkChannel();

    static int nonmavlinkCount = 0;
    static bool warnedUser = false;
    static bool checkedUserNonMavlink = false;
    static bool warnedUserNonMavlink = false;

    for (int position = 0; position < b.size(); position++)
    {
        unsigned int decodeState = mavlink_parse_char(mavlinkChannel, (uint8_t)(b[position]), &message, &status);
        if (decodeState == 0 && !link->decodedFirstMavlinkPacket())
        {
            nonmavlinkCount++;
            if (nonmavlinkCount > 2000 && !warnedUserNonMavlink)
            {
                //2000 bytes with no mavlink message. Are we connected to a mavlink capable device?
                if (!checkedUserNonMavlink)
                {
                    link->requestReset();
                    checkedUserNonMavlink = true;
                }
                else
                {
                    warnedUserNonMavlink = true;
                    emit protocolStatusMessage(tr("MAVLink Protocol"), tr("There is a MAVLink Version or Baud Rate Mismatch. "
                                                                          "Please check if the baud rates of QGroundControl and your autopilot are the same."));
                }
            }
        }
        if (decodeState == 1)
        {
            if (!link->decodedFirstMavlinkPacket()) 
                link->setDecodedFirstMavlinkPacket(true);
			
            if(message.msgid == MAVLINK_MSG_ID_PING)
            {
                // process ping requests (tgt_system and tgt_comp must be zero)
                mavlink_ping_t ping;
                mavlink_msg_ping_decode(&message, &ping);
                if(!ping.target_system && !ping.target_component)
                {
                    mavlink_message_t msg;
                    mavlink_msg_ping_pack(getSystemId(), getComponentId(), &msg, ping.time_usec, ping.seq, message.sysid, message.compid);
                    _sendMessage(msg);
                }
            }

            if(message.msgid == MAVLINK_MSG_ID_RADIO_STATUS)
            {
                // process telemetry status message
                mavlink_radio_status_t rstatus;
                mavlink_msg_radio_status_decode(&message, &rstatus);
                int rssi = rstatus.rssi,
                    remrssi = rstatus.remrssi;
                // 3DR Si1k radio needs rssi fields to be converted to dBm
                if (message.sysid == '3' && message.compid == 'D') {
                    /* Per the Si1K datasheet figure 23.25 and SI AN474 code
                     * samples the relationship between the RSSI register
                     * and received power is as follows:
                     *
                     *                       10
                     * inputPower = rssi * ------ 127
                     *                       19
                     *
                     * Additionally limit to the only realistic range [-120,0] dBm
                     */
                    rssi    = qMin(qMax(qRound(static_cast<qreal>(rssi)    / 1.9 - 127.0), - 120), 0);
                    remrssi = qMin(qMax(qRound(static_cast<qreal>(remrssi) / 1.9 - 127.0), - 120), 0);
                }

                emit radioStatusChanged(link, rstatus.rxerrors, rstatus.fixed, rssi, remrssi,
                    rstatus.txbuf, rstatus.noise, rstatus.remnoise);
            }
#ifndef __mobile__
            // Log data
            if (!_logSuspendError && !_logSuspendReplay && _LogFile && _LogFile->isOpen())
            {
                uint8_t buf[MAVLINK_MAX_PACKET_LEN+sizeof(quint64)];

                // Write the uint64 time in microseconds in big endian format before the message.
                // This timestamp is saved in UTC time. We are only saving in ms precision because
                // getting more than this isn't possible with Qt without a ton of extra code.
                quint64 time = (quint64)QDateTime::currentMSecsSinceEpoch() * 1000;
                qToBigEndian(time, buf);

                // Then write the message to the buffer
                int len = mavlink_msg_to_send_buffer(buf + sizeof(quint64), &message);

                // Determine how many bytes were written by adding the timestamp size to the message size
                len += sizeof(quint64);

                // Now write this timestamp/message pair to the log.
                QByteArray ba((const char*)buf, len);
                if (_LogFile->write(ba) != len)
                {
                    qDebug() << "MAVLinkProtocol::receiveBytes:write err, mav file= " << _LogFile->fileName();
                    // If there's an error logging data, raise an alert and stop logging.
                    emit protocolStatusMessage(tr("MAVLink Protocol"), tr("MAVLink Logging failed. Could not write to file %1, logging disabled.").arg(_LogFile->fileName()));
                    _stopLogging();
                    _logSuspendError = true;
                }
                _LogFile->flush();

                // Check for the vehicle arming going by. This is used to trigger log save.
                if (!_logPromptForSave && message.msgid == MAVLINK_MSG_ID_HEARTBEAT)
                {
                    mavlink_heartbeat_t state;
                    mavlink_msg_heartbeat_decode(&message, &state);
                    if (state.base_mode & MAV_MODE_FLAG_DECODE_POSITION_SAFETY) {
                        _logPromptForSave = true;
                    }
                }
            }
#endif
            if (message.msgid == MAVLINK_MSG_ID_HEARTBEAT)
            {
#ifndef __mobile__
                // Start loggin on first heartbeat
                _startLogging(_filename);
#endif

                _linkMgr->setConnectUAVStatus(true);
                mavlink_heartbeat_t heartbeat;
                mavlink_msg_heartbeat_decode(&message, &heartbeat);
                emit vehicleHeartbeatInfo(link, message.sysid, heartbeat.mavlink_version, heartbeat.autopilot, heartbeat.type);
            }

            // Increase receive counter
            totalReceiveCounter[mavlinkChannel]++;
            currReceiveCounter[mavlinkChannel]++;

            // Determine what the next expected sequence number is, accounting for
            // never having seen a message for this system/component pair.
            int lastSeq = lastIndex[message.sysid][message.compid];
            int expectedSeq = (lastSeq == -1) ? message.seq : (lastSeq + 1);

            // And if we didn't encounter that sequence number, record the error
            if (message.seq != expectedSeq)
            {
                // Determine how many messages were skipped
                int lostMessages = message.seq - expectedSeq;

                // Out of order messages or wraparound can cause this, but we just ignore these conditions for simplicity
                if (lostMessages < 0)
                {
                    lostMessages = 0;
                }

                // And log how many were lost for all time and just this timestep
                totalLossCounter[mavlinkChannel] += lostMessages;
                currLossCounter[mavlinkChannel] += lostMessages;
            }

            // And update the last sequence number for this system/component pair
            lastIndex[message.sysid][message.compid] = expectedSeq;

            // Update on every 32th packet
            if ((totalReceiveCounter[mavlinkChannel] & 0x1F) == 0)
            {
                // Calculate new loss ratio
                // Receive loss
                float receiveLoss = (double)currLossCounter[mavlinkChannel]/(double)(currReceiveCounter[mavlinkChannel]+currLossCounter[mavlinkChannel]);
                receiveLoss *= 100.0f;
                currLossCounter[mavlinkChannel] = 0;
                currReceiveCounter[mavlinkChannel] = 0;
                emit receiveLossChanged(message.sysid, receiveLoss);
            }
            // The packet is emitted as a whole, as it is only 255 - 261 bytes short
            // kind of inefficient, but no issue for a groundstation pc.
            // It buys as reentrancy for the whole code over all threads
            emit messageReceived(link, message);
        }
    }
}

/**
 * @return The name of this protocol
 **/
QString MAVLinkProtocol::getName()
{
    return QString(tr("MAVLink protocol"));
}

/** @return System id of this application */
int MAVLinkProtocol::getSystemId()const
{
    return systemId;
}

void MAVLinkProtocol::setSystemId(int id)
{
    systemId = id;
    storeSettings();
}

/** @return Component id of this application */
int MAVLinkProtocol::getComponentId()const
{
    return componentId;//BASE::defaultComponentId;
}

void MAVLinkProtocol::setComponentId(int id)
{
    componentId = id;
    storeSettings();
}

void MAVLinkProtocol::_sendMessage(mavlink_message_t message)
{
    for (LinkCommand *itr : _linkMgr->linkCmds())
    {
        _sendMessage(itr->link(), message);
    }
}

/**
 * @param link the link to send the message over
 * @param message message to send
 */
void MAVLinkProtocol::_sendMessage(LinkInterface* link, mavlink_message_t message)
{
    // Create buffer
    static uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    // Rewriting header to ensure correct link ID is set
    if (const mavlink_msg_entry_t *entry = mavlink_get_msg_entry(message.msgid))
    {
        mavlink_finalize_message_chan(&message, this->getSystemId(), this->getComponentId(), link->getMavlinkChannel(), 0, message.len, entry->crc_extra);
        // Write message into buffer, prepending start sign
        int len = mavlink_msg_to_send_buffer(buffer, &message);
        // If link is connected
        if (link->isConnected())
            link->writeBytes(QByteArray((const char*)buffer, len));
    }
}

/**
 * @param link the link to send the message over
 * @param message message to send
 * @param systemid id of the system the message is originating from
 * @param componentid id of the component the message is originating from
 */
void MAVLinkProtocol::_sendMessage(LinkInterface* link, mavlink_message_t message, quint8 systemid, quint8 componentid)
{
    if (const mavlink_msg_entry_t *entry = mavlink_get_msg_entry(message.msgid))
    {
        // Create buffer
        static uint8_t buffer[MAVLINK_MAX_PACKET_LEN]; 
        // Rewriting header to ensure correct link ID is set
        mavlink_finalize_message_chan(&message, systemid, componentid, link->getMavlinkChannel(), 0, message.len, entry->crc_extra);
        // Write message into buffer, prepending start sign
        int len = mavlink_msg_to_send_buffer(buffer, &message);
        // If link is connected
        if (link->isConnected())
        {
            link->writeBytes(QByteArray((char*)buffer, len));
        }
    }
}

void MAVLinkProtocol::enableMultiplexing(bool enabled)
{
    bool changed = false;
    if (enabled != m_multiplexingEnabled) changed = true;

    m_multiplexingEnabled = enabled;
    if (changed) {
        emit multiplexingChanged(m_multiplexingEnabled);
    }
}

void MAVLinkProtocol::enableAuth(bool enable)
{
    bool changed = false;
    m_authEnabled = enable;
    if (m_authEnabled != enable) {
        changed = true;
    }
    if (changed) {
        emit authChanged(m_authEnabled);
    }
}

void MAVLinkProtocol::enableParamGuard(bool enabled)
{
    if (enabled != m_paramGuardEnabled) {
        m_paramGuardEnabled = enabled;
        emit paramGuardChanged(m_paramGuardEnabled);
    }
}

void MAVLinkProtocol::enableActionGuard(bool enabled)
{
    if (enabled != m_actionGuardEnabled) {
        m_actionGuardEnabled = enabled;
        emit actionGuardChanged(m_actionGuardEnabled);
    }
}

void MAVLinkProtocol::setParamRetransmissionTimeout(int ms)
{
    if (ms != m_paramRetransmissionTimeout) {
        m_paramRetransmissionTimeout = ms;
        emit paramRetransmissionTimeoutChanged(m_paramRetransmissionTimeout);
    }
}

void MAVLinkProtocol::setParamRewriteTimeout(int ms)
{
    if (ms != m_paramRewriteTimeout) {
        m_paramRewriteTimeout = ms;
        emit paramRewriteTimeoutChanged(m_paramRewriteTimeout);
    }
}

void MAVLinkProtocol::setActionRetransmissionTimeout(int ms)
{
    if (ms != m_actionRetransmissionTimeout) {
        m_actionRetransmissionTimeout = ms;
        emit actionRetransmissionTimeoutChanged(m_actionRetransmissionTimeout);
    }
}

void MAVLinkProtocol::enableVersionCheck(bool enabled)
{
    m_enable_version_check = enabled;
    emit versionCheckChanged(enabled);
}

#ifndef __mobile__
void MAVLinkProtocol::_startLogging(const QString& filename)
{
    if(_filename.length() > 1 && _filename != filename){
        _stopLogging();
    }
    _filename = filename;
    if (_LogFile && _LogFile->isOpen())
    {
        return;
    }
    _stopLogging();

    Q_ASSERT_X(_LogFile == NULL, "startLogging", "_LogFile == NULL");

    _LogFile = new QFile(_filename);
    if (_LogFile->open(QIODevice::WriteOnly | QIODevice::Append)){

    } else {
        qDebug() << "MAVLinkProtocol::_startLogging:open err, mav file= " << _filename;
        delete _LogFile;
        _LogFile = NULL;
    }
    return ;
}

void MAVLinkProtocol::_stopLogging(void)
{
    if (_LogFile && _LogFile->isOpen()){

        // Close the current open file
        _LogFile->close();
        delete _LogFile;
        _LogFile = NULL;
    }
    //m_loggingEnabled = false;
    //pack_len = 0;

    if(_LogFile && _LogFile->size() == 0){
        QFile::remove(_filename);
    }
}
#endif
