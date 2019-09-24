#include "MissionManager.h"
#include "VGVehicle.h"
#include "MAVLinkProtocol.h"
#include "VGApplication.h"
#include "VGToolBox.h"
#include <QLoggingCategory>
#include "VGGlobalFun.h"
#include "VGMavLinkCode.h"

static QLoggingCategory sMissionManagerLog("sMissionManagerLog");
static const int _ackTimeoutMilliseconds = 1000;
static const int _maxRetryCount = 5;

MissionManager::MissionManager(VGVehicle* vehicle): m_vehicle(vehicle), m_dedicatedLink(NULL)
, m_ackTimeoutTimer(NULL), m_expectedAck(AckNone)
, m_transactionInProgress(TransactionNone)
, m_resumeMission(false), m_lastMissionRequest(-1)
, m_currentMissionIndex(-1), m_lastCurrentIndex(-1)
, m_bSendHomePosition(false), m_bHasValidMission(false)
, m_typeMissionWrite(MAV_MISSION_TYPE_MISSION)
, m_typeMissionRead(MAV_MISSION_TYPE_MISSION)
{
    connect(m_vehicle, &VGVehicle::mavlinkMessageReceived, this, &MissionManager::_mavlinkMessageReceived);
    connect(m_vehicle, &VGVehicle::connectionLostChanged, this, &MissionManager::onConnectiontChanged);
    
    m_ackTimeoutTimer = new QTimer(this);
    m_ackTimeoutTimer->setSingleShot(true);
    m_ackTimeoutTimer->setInterval(_ackTimeoutMilliseconds);
    
    connect(m_ackTimeoutTimer, &QTimer::timeout, this, &MissionManager::_ackTimeout);
}

MissionManager::~MissionManager()
{
}

void MissionManager::_writeMissionItemsWorker(void)
{
    m_lastMissionRequest = -1;

    emit progressPct(0);
    m_bDown = false;

    qCDebug(sMissionManagerLog) << "writeMissionItems count:" << m_writeMissionItems.count();

    // Prime write list
    for (int i=0; i<m_writeMissionItems.count(); i++) {
        m_itemIndicesToWrite << i;
    }

    m_transactionInProgress = TransactionWrite;
    m_retryCount = 0;
    emit inProgressChanged(true);
    _writeMissionCount();

    m_currentMissionIndex = -1;
    m_lastCurrentIndex = -1;
    emit currentIndexChanged(-1);
    emit lastCurrentIndexChanged(-1);
}

void MissionManager::writeMissionItems(const QList<MissionItem*>& items, bool skipFirstItem)
{
    if (inProgress() || items.isEmpty())
        return;

	m_bSendHomePosition = true;
    _clearAndDeleteWriteMissionItems();

    int firstIndex = skipFirstItem ? 1 : 0;
    m_typeMissionWrite = items.first()->missionType();
    for (MissionItem *itr : items)
    {
        MissionItem* item = new MissionItem(*itr);
        m_writeMissionItems.append(item);
        item->setIsCurrentItem(itr->sequenceNumber() == firstIndex);
        if (skipFirstItem)
        {
            item->setSequenceNumber(item->sequenceNumber() - 1);
            if (item->command() == MAV_CMD_DO_JUMP)
                item->setParam1(item->param1().toInt() - 1);
        }
    }
    _writeMissionItemsWorker();
}

/// This begins the write sequence with the vehicle. This may be called during a retry.
void MissionManager::_writeMissionCount(void)
{
    if (m_dedicatedLink = m_vehicle->priorityLink())
    {
        mavlink_message_t message;
        VGMavLinkCode::EncodeMissionCount(message, m_writeMissionItems.count(), (MAV_MISSION_TYPE)m_typeMissionWrite,
            qvgApp->toolbox()->mavlinkProtocol(), m_dedicatedLink->getMavlinkChannel(), MAV_COMP_ID_MISSIONPLANNER, m_vehicle->id());
        m_vehicle->sendMessageOnLink(m_dedicatedLink, message);
        _startAckTimeout(AckMissionRequest);
    }
}

void MissionManager::writeArduPilotGuidedMissionItem(const QGeoCoordinate& gotoCoord, bool altChangeOnly)
{
    if (inProgress()) {
        qCDebug(sMissionManagerLog) << "writeArduPilotGuidedMissionItem called while transaction in progress";
        return;
    }

    m_transactionInProgress = TransactionWrite;

    mavlink_message_t       messageOut;
    mavlink_mission_item_t  missionItem;

    memset(&missionItem, 8, sizeof(missionItem));
    missionItem.target_system =     m_vehicle->id();
	missionItem.target_component =	qvgApp->toolbox()->mavlinkProtocol()->getComponentId();
    missionItem.seq =               0;
    missionItem.command =           MAV_CMD_NAV_WAYPOINT;
    missionItem.param1 =            0;
    missionItem.param2 =            0;
    missionItem.param3 =            0;
    missionItem.param4 =            0;
    missionItem.x =                 gotoCoord.latitude();
    missionItem.y =                 gotoCoord.longitude();
    missionItem.z =                 gotoCoord.altitude();
    missionItem.frame =             MAV_FRAME_GLOBAL_RELATIVE_ALT;
    missionItem.current =           altChangeOnly ? 3 : 2;
    missionItem.autocontinue =      true;

    m_dedicatedLink = m_vehicle->priorityLink();
	mavlink_msg_mission_item_encode_chan(qvgApp->toolbox()->mavlinkProtocol()->getSystemId(),
										 qvgApp->toolbox()->mavlinkProtocol()->getComponentId(),
                                         m_dedicatedLink->getMavlinkChannel(),
                                         &messageOut,
                                         &missionItem);

    m_vehicle->sendMessageOnLink(m_dedicatedLink, messageOut);
    _startAckTimeout(AckGuidedItem);
    emit inProgressChanged(true);
}

void MissionManager::loadFromVehicle(MAV_MISSION_TYPE tp)
{
    qCDebug(sMissionManagerLog) << "loadFromVehicle read sequence";

    if (inProgress()) {
        qCDebug(sMissionManagerLog) << "loadFromVehicle called while transaction in progress";
        return;
    }

    m_typeMissionRead = tp;
    m_retryCount = 0;
    m_transactionInProgress = TransactionRead;
    emit inProgressChanged(true);
    _requestList();
}

/// Internal call to request list of mission items. May be called during a retry sequence.
void MissionManager::_requestList()
{
    qCDebug(sMissionManagerLog) << "_requestList retry count" << m_retryCount;

    mavlink_message_t               message;
    mavlink_mission_request_list_t  request;

    memset(&request, 0, sizeof(request));

    m_itemIndicesToRead.clear();
    _clearMissionItems();

    request.target_system = m_vehicle->id();
    request.target_component = MAV_COMP_ID_MISSIONPLANNER;
    request.mission_type = m_typeMissionRead;

    if (m_dedicatedLink = m_vehicle->priorityLink())
    {
        mavlink_msg_mission_request_list_encode_chan(qvgApp->toolbox()->mavlinkProtocol()->getSystemId(),
            qvgApp->toolbox()->mavlinkProtocol()->getComponentId(),
            m_dedicatedLink->getMavlinkChannel(),
            &message,
            &request);

        m_vehicle->sendMessageOnLink(m_dedicatedLink, message);
        m_bHasValidMission = false;
        _startAckTimeout(AckMissionCount);
    }
}

void MissionManager::_ackTimeout(void)
{
    if (m_expectedAck == AckNone)
        return;

    switch (m_expectedAck)
    {
    case AckNone:
        qCWarning(sMissionManagerLog) << "_ackTimeout timeout with AckNone";
        _sendError(InternalError, QString::fromStdWString(L"任务通讯失败"));
        break;
    case AckMissionCount:
        // MISSION_COUNT message expected
        if (m_retryCount > _maxRetryCount) {
            _sendError(VehicleError, QString::fromStdWString(L"下载航线失败, 重试超限"));
            _finishTransaction(false);
        } else {
            m_retryCount++;
            qCDebug(sMissionManagerLog) << "Retrying REQUEST_LIST retry Count" << m_retryCount;
            _requestList();
        }
        break;
    case AckMissionItem:
        // MISSION_ITEM expected
        if (m_retryCount > _maxRetryCount) {
            _sendError(VehicleError, QString::fromStdWString(L"下载航线失败"));
            _finishTransaction(false);
        } else {
            m_retryCount++;
            qCDebug(sMissionManagerLog) << "Retrying MISSION_REQUEST retry Count" << m_retryCount;
            _requestNextMissionItem();
        }
        break;
    case AckMissionRequest:
        // MISSION_REQUEST is expected, or MISSION_ACK to end sequence
        if (m_itemIndicesToWrite.count() == 0) {
            // Vehicle did not send final MISSION_ACK at end of sequence
            _sendError(VehicleError, QString::fromStdWString(L"上传航线失败"));
            _finishTransaction(false);
        } else if (m_itemIndicesToWrite[0] == 0) {
            // Vehicle did not respond to MISSION_COUNT, try again
            if (m_retryCount > _maxRetryCount) {
                _sendError(VehicleError, QString::fromStdWString(L"尝试上传次数过多"));
                _finishTransaction(false);
            } else {
                m_retryCount++;
                qCDebug(sMissionManagerLog) << "Retrying MISSION_COUNT retry Count" << m_retryCount;
                _writeMissionCount();
            }
        } else {
            // Vehicle did not request all items from ground station
            _sendError(AckTimeoutError, QString::fromStdWString(L"设备没有读取所有任务点"));
            m_expectedAck = AckNone;
            _finishTransaction(false);
        }
        break;
    case AckMissionClearAll:
        // MISSION_ACK expected
        if (m_retryCount > _maxRetryCount) {
            _sendError(VehicleError, QString::fromStdWString(L"删除任务次数超限"));
            _finishTransaction(false);
        } else {
            m_retryCount++;
            qCDebug(sMissionManagerLog) << "Retrying MISSION_CLEAR_ALL retry Count" << m_retryCount;
            removeAllWorker();
        }
        break;
    case AckGuidedItem:
        // MISSION_REQUEST is expected, or MISSION_ACK to end sequence
    default:
        _sendError(AckTimeoutError, QString::fromStdWString(L"上传航线失败，设备没有回应任务点"));
        m_expectedAck = AckNone;
        _finishTransaction(false);
    }
}

void MissionManager::onConnectiontChanged(bool bLost)
{
    if (!bLost)
    {
        if (m_writeMissionItems.count())
            _writeMissionItemsWorker();
    }
}

void MissionManager::_startAckTimeout(AckType_t ack)
{
    m_expectedAck = ack;
    m_ackTimeoutTimer->start();
}

/// Checks the received ack against the expected ack. If they match the ack timeout timer will be stopped.
/// @return true: received ack matches expected ack
bool MissionManager::_checkForExpectedAck(AckType_t receivedAck)
{
    if (receivedAck == m_expectedAck) {
        m_expectedAck = AckNone;
        m_ackTimeoutTimer->stop();
        return true;
    } else {
        if (m_expectedAck == AckNone) {
            // Don't worry about unexpected mission commands, just ignore them; ArduPilot updates home position using
            // spurious MISSION_ITEMs.
        } else {
            // We just warn in this case, this could be crap left over from a previous transaction or the vehicle going bonkers.
            // Whatever it is we let the ack timeout handle any error output to the user.
            qCDebug(sMissionManagerLog) << QString("Out of sequence ack expected:received %1:%2").arg(_ackTypeToString(m_expectedAck)).arg(_ackTypeToString(receivedAck));
        }
        return false;
    }
}

void MissionManager::_readTransactionComplete(void)
{
    mavlink_message_t       message;
    mavlink_mission_ack_t   missionAck = { 0 };
    
    missionAck.target_system =      m_vehicle->id();
    missionAck.target_component =   MAV_COMP_ID_MISSIONPLANNER;
    missionAck.type =               MAV_MISSION_ACCEPTED;
    missionAck.mission_type =       m_typeMissionRead;
    
    mavlink_msg_mission_ack_encode_chan(qvgApp->toolbox()->mavlinkProtocol()->getSystemId(),
                                        qvgApp->toolbox()->mavlinkProtocol()->getComponentId(),
                                        m_dedicatedLink->getMavlinkChannel(),
                                        &message,
                                        &missionAck);
    
    m_vehicle->sendMessageOnLink(m_dedicatedLink, message);
    _finishTransaction(true);
}

void MissionManager::_handleMissionCount(const mavlink_message_t& message)
{
    mavlink_mission_count_t missionCount;
    
    if (!_checkForExpectedAck(AckMissionCount))
        return;

    m_retryCount = 0;
    
    mavlink_msg_mission_count_decode(&message, &missionCount);
    qCDebug(sMissionManagerLog) << "_handleMissionCount count:" << missionCount.count;

    if (missionCount.count == 0)
    {
        _readTransactionComplete();
    }
    else
    {
        // Prime read list
        for (int i=0; i<missionCount.count; i++)
        {
            m_itemIndicesToRead << i;
        }
        _requestNextMissionItem();
    }
}

void MissionManager::_requestNextMissionItem(void)
{
    if (m_itemIndicesToRead.count() == 0)
        return;

    mavlink_message_t message;
    VGMavLinkCode::EncodeReadItem(message, m_itemIndicesToRead[0], m_vehicle->supportsMissionItemInt(), m_typeMissionRead
        , qvgApp->mavLink(), m_dedicatedLink->getMavlinkChannel(), m_vehicle->id()); 
    m_vehicle->sendMessageOnLink(m_dedicatedLink, message);
    _startAckTimeout(AckMissionItem);
}

void MissionManager::_handleMissionItem(const mavlink_message_t& message)
{
    MissionItem* item = new MissionItem(this);
    if (!item)
        return;

    if (VGMavLinkCode::DecodeMissionItem(message, *item))
    {
        if (item->frame() == MAV_FRAME_GLOBAL_INT)
            item->setFrame(MAV_FRAME_GLOBAL);
        else if (item->frame() == MAV_FRAME_GLOBAL_RELATIVE_ALT_INT)
            item->setFrame(MAV_FRAME_GLOBAL_RELATIVE_ALT);


        bool ardupilotHomePositionUpdate = false;
        if (!_checkForExpectedAck(AckMissionItem))
        {
            if (m_vehicle->firewareType() && item->sequenceNumber() == 0)
                ardupilotHomePositionUpdate = true;
            else
                return;
        }

        if (m_itemIndicesToRead.contains(item->sequenceNumber()))
        {
            m_itemIndicesToRead.removeOne(item->sequenceNumber());
            item->setCoordinate(VGGlobalFunc::gpsCorrect(item->coordinate()));
            m_missionItems.append(item);
        }
        else
        {
            _startAckTimeout(AckMissionItem);
            return;
        }

        m_bDown = true;
        emit progressPct(double(item->sequenceNumber()) / m_missionItems.count());
        m_retryCount = 0;
        if (m_itemIndicesToRead.count() == 0)
            _readTransactionComplete();
        else
            _requestNextMissionItem();
    }
}

void MissionManager::_clearMissionItems(void)
{
    m_itemIndicesToRead.clear();
    _clearAndDeleteMissionItems();
}

void MissionManager::_handleMissionRequest(const mavlink_message_t& message, bool missionItemInt)
{    
    if (!_checkForExpectedAck(AckMissionRequest))
        return;

    int seq = VGMavLinkCode::DecodeMissionRequest(message);
    if (seq<0 || seq > m_writeMissionItems.count() - 1)
    {
        _sendError(RequestRangeError, tr("Vehicle requested item outside range, count:request %1:%2. Send to Vehicle failed.")
            .arg(m_writeMissionItems.count()).arg(seq));
        _finishTransaction(false);
        return;
    }

    m_bDown = false;
    emit progressPct(double(seq) / m_writeMissionItems.count());
    m_lastMissionRequest = seq;
    if (!m_itemIndicesToWrite.contains(seq))
        qCDebug(sMissionManagerLog) << "_handleMissionRequest sequence number requested which has already been sent, sending again:" << seq;
    else 
        m_itemIndicesToWrite.removeOne(seq);
    
    MissionItem* item = m_writeMissionItems[seq];
    mavlink_message_t   messageOut;
    if (missionItemInt)
        VGMavLinkCode::EncodeMissionItemInt(messageOut, *item, seq==0,
            qvgApp->mavLink(), m_dedicatedLink->getMavlinkChannel(),
            m_vehicle->id());
    else
        VGMavLinkCode::EncodeMissionItem(messageOut, *item, seq == 0,
            qvgApp->mavLink(), m_dedicatedLink->getMavlinkChannel(),
            m_vehicle->id());
    
    m_vehicle->sendMessageOnLink(m_dedicatedLink, messageOut);
    _startAckTimeout(AckMissionRequest);
}

void MissionManager::_handleMissionAck(const mavlink_message_t& message)
{
    AckType_t savedExpectedAck = m_expectedAck;
    if (!_checkForExpectedAck(m_expectedAck))
        return;

	mavlink_mission_ack_t missionAck;
    mavlink_msg_mission_ack_decode(&message, &missionAck);
	QString strTmp = missionResultToString((MAV_MISSION_RESULT)missionAck.type);
    
	if (missionAck.type != MAV_MISSION_ACCEPTED)
		qCDebug(sMissionManagerLog) << "_handleMissionAck type:" << strTmp;

    switch (savedExpectedAck)
    {
    case AckNone:
        // State machine is idle. Vehicle is confused.
        _sendError(VehicleError, QString::fromStdWString(L"任务异常回应--%1").arg(strTmp));
        break;
    case AckMissionCount:
        // MISSION_COUNT message expected
        _sendError(VehicleError, QString::fromStdWString(L"设备返回错误--%1").arg(strTmp));
        _finishTransaction(false);
        break;
    case AckMissionItem:
        // MISSION_ITEM expected
        _sendError(VehicleError, QString::fromStdWString(L"设备返回错误--%1").arg(strTmp));
        _finishTransaction(false);
        break;
    case AckMissionRequest:
        // MISSION_REQUEST is expected, or MISSION_ACK to end sequence
        if (missionAck.type == MAV_MISSION_ACCEPTED)
        {
            if (m_itemIndicesToWrite.count() == 0)
            {
                qCDebug(sMissionManagerLog) << "_handleMissionAck write sequence complete";
                _finishTransaction(true);
            }
            else
            {
                _sendError(MissingRequestsError, QString::fromStdWString(L"上传航线失败，读取第%1点中断").arg(m_itemIndicesToWrite.count()));
                _finishTransaction(false);
            }
        }
        else
        {
            _sendError(VehicleError, QString::fromStdWString(L"设备返回错误--%1").arg(strTmp));
            _finishTransaction(false);
        }
        break;
    case AckMissionClearAll:
        // MISSION_ACK expected
        if (missionAck.type != MAV_MISSION_ACCEPTED)
            _sendError(VehicleError, QString::fromStdWString(L"设备返回错误--%1，删除失败！").arg(strTmp));
        _finishTransaction(missionAck.type == MAV_MISSION_ACCEPTED);
        break;
    case AckGuidedItem:
        // MISSION_REQUEST is expected, or MISSION_ACK to end sequence
        if (missionAck.type == MAV_MISSION_ACCEPTED) {
			qCDebug(sMissionManagerLog) << "_handleMissionAck guided mode item accepted";
			_finishTransaction(true);
		}else {
            _sendError(VehicleError, QString::fromStdWString(L"设备返回错误--%1，不接收引导Item！").arg(strTmp));
            _finishTransaction(false);
        }
        break;
    }
}
/// Called when a new mavlink message for out vehicle is received
void MissionManager::_mavlinkMessageReceived(const mavlink_message_t& message)
{
    switch (message.msgid) {
    case MAVLINK_MSG_ID_MISSION_COUNT:
        _handleMissionCount(message);
        break;
    case MAVLINK_MSG_ID_MISSION_ITEM:
    case MAVLINK_MSG_ID_MISSION_ITEM_INT:
        _handleMissionItem(message);
        break;
    case MAVLINK_MSG_ID_MISSION_REQUEST:
        _handleMissionRequest(message, false /* missionItemInt */);
        break;
    case MAVLINK_MSG_ID_MISSION_REQUEST_INT:
        _handleMissionRequest(message, true /* missionItemInt */);
        break;

    case MAVLINK_MSG_ID_MISSION_ACK:
        _handleMissionAck(message);
        break;
    case MAVLINK_MSG_ID_MISSION_ITEM_REACHED:
        // FIXME: NYI
        break;
    case MAVLINK_MSG_ID_MISSION_CURRENT:
        _handleMissionCurrent(message);
        break;
    }
}

void MissionManager::_sendError(ErrorCode_t errorCode, const QString& errorMsg)
{
    qCDebug(sMissionManagerLog) << "Sending error" << errorCode << errorMsg;

    emit error(errorCode, errorMsg);
}

QString MissionManager::_ackTypeToString(AckType_t ackType)
{
    switch (ackType) {
    case AckNone:
        return QString("No Ack");
    case AckMissionCount:
        return QString("MISSION_COUNT");
    case AckMissionItem:
        return QString("MISSION_ITEM");
    case AckMissionRequest:
        return QString("MISSION_REQUEST");
    case AckGuidedItem:
        return QString("Guided Mode Item");
    default:
        //qWarning(sMissionManagerLog) << "Fell off end of switch statement";
        return QString("QGC Internal Error");
    }
}

QString MissionManager::_lastMissionReqestString(MAV_MISSION_RESULT result)
{
    if (m_lastMissionRequest != -1 && m_lastMissionRequest >= 0 && m_lastMissionRequest < m_writeMissionItems.count())
    {
        MissionItem* item = m_writeMissionItems[m_lastMissionRequest];

        switch (result)
        {
        case MAV_MISSION_UNSUPPORTED_FRAME:
            return QString(". Frame: %1").arg(item->frame());
        case MAV_MISSION_INVALID_PARAM1:
            return QString(". Param1: %1").arg(item->param1().toDouble());
        case MAV_MISSION_INVALID_PARAM2:
            return QString(". Param2: %1").arg(item->param2().toDouble());
        case MAV_MISSION_INVALID_PARAM3:
            return QString(". Param3: %1").arg(item->param3().toBool());
        case MAV_MISSION_INVALID_PARAM4:
            return QString(". Param4: %1").arg(item->param4().toDouble());
        case MAV_MISSION_INVALID_PARAM5_X:
            return QString(". Param5: %1").arg(item->param5().toDouble());
        case MAV_MISSION_INVALID_PARAM6_Y:
            return QString(". Param6: %1").arg(item->param6().toDouble());
        case MAV_MISSION_INVALID_PARAM7:
            return QString(". Param7: %1").arg(item->param7().toDouble());
        case MAV_MISSION_INVALID_SEQUENCE:
            return QString(". Sequence: %1").arg(item->sequenceNumber());
        default:
            break;
        }
    }

    return QString();
}

QString MissionManager::missionResultToString(MAV_MISSION_RESULT result)
{
    QString resultString;
    switch (result) {
    case MAV_MISSION_ACCEPTED:
        resultString = QString::fromStdWString(L"任务接收");
        break;
    case MAV_MISSION_ERROR:
        resultString = QString::fromStdWString(L"任务错误");
        break;
    case MAV_MISSION_UNSUPPORTED_FRAME:
        resultString = QString::fromStdWString(L"任务FRAME类型不支持");
        break;
    case MAV_MISSION_UNSUPPORTED:
        resultString = QString::fromStdWString(L"任务COMMAND类型不支持");
        break;
    case MAV_MISSION_NO_SPACE:
        resultString = QString::fromStdWString(L"没有存储空间");
        break;
    case MAV_MISSION_INVALID:
        resultString = QString::fromStdWString(L"有参数无效");
        break;
    case MAV_MISSION_INVALID_PARAM1:
        resultString = QString::fromStdWString(L"PARAM1无效");
        break;
    case MAV_MISSION_INVALID_PARAM2:
        resultString = QString::fromStdWString(L"PARAM2无效");
        break;
    case MAV_MISSION_INVALID_PARAM3:
        resultString = QString::fromStdWString(L"PARAM3无效");
        break;
    case MAV_MISSION_INVALID_PARAM4:
        resultString = QString::fromStdWString(L"PARAM4无效");
        break;
    case MAV_MISSION_INVALID_PARAM5_X:
        resultString = QString::fromStdWString(L"PARAM5无效");
        break;
    case MAV_MISSION_INVALID_PARAM6_Y:
        resultString = QString::fromStdWString(L"PARAM6无效");
        break;
    case MAV_MISSION_INVALID_PARAM7:
        resultString = QString::fromStdWString(L"PARAM7无效");
        break;
    case MAV_MISSION_INVALID_SEQUENCE:
        resultString = QString::fromStdWString(L"任务点序列超限");
        break;
    case MAV_MISSION_DENIED:
        resultString = QString::fromStdWString(L"设备禁止接收任务");
        break;
    default:
        resultString = QString("地面站内部错误");
    }

    return resultString;
}

void MissionManager::_finishTransaction(bool success)
{
    emit progressPct(1, success);

    m_itemIndicesToRead.clear();
    m_itemIndicesToWrite.clear();

    // First thing we do is clear the transaction. This way inProgesss is off when we signal transaction complete.
    TransactionType_t currentTransactionType = m_transactionInProgress;
    m_transactionInProgress = TransactionNone;
    if (currentTransactionType != TransactionNone)
    {
        m_transactionInProgress = TransactionNone;
        qDebug() << "inProgressChanged";
        emit inProgressChanged(false);
    }

    switch (currentTransactionType)
    {
    case TransactionRead:
        if (!success)     
            _clearAndDeleteMissionItems();// Read from vehicle failed, clear partial list
        m_bHasValidMission = true;
        emit newMissionItemsAvailable(false, m_typeMissionRead);
        break;
    case TransactionWrite:
        if (success) {
            // Write succeeded, update internal list to be current
            _clearAndDeleteMissionItems();
            for (int i=0; i<m_writeMissionItems.count(); i++) {
                m_missionItems.append(m_writeMissionItems[i]);
            }
            m_writeMissionItems.clear();
            emit newMissionItemsAvailable(false, m_typeMissionWrite);
        } else {
            // Write failed, throw out the write list
            _clearAndDeleteWriteMissionItems();
        }
        emit sendComplete(!success /* error */);
        break;
    case TransactionRemoveAll:
        emit removeAllComplete(!success /* error */);
        break;
    default:
        break;
    }

    if (m_resumeMission) {
        m_resumeMission = false;
        emit resumeMissionReady();
    }
}

bool MissionManager::inProgress(void)
{
    return m_transactionInProgress != TransactionNone;
}

MAV_MISSION_TYPE MissionManager::typeOfReadMission() const
{
    return m_typeMissionRead;
}

const QList<MissionItem*> &MissionManager::missionItems(void) const
{
    return m_missionItems;
}

void MissionManager::_handleMissionCurrent(const mavlink_message_t& message)
{
    mavlink_mission_current_t missionCurrent;
    mavlink_msg_mission_current_decode(&message, &missionCurrent);
    if (missionCurrent.seq != m_currentMissionIndex)
	{
        m_currentMissionIndex = missionCurrent.seq;
        emit currentIndexChanged(m_currentMissionIndex);
    }

	if (m_currentMissionIndex != m_lastCurrentIndex)
	{
        m_lastCurrentIndex = m_currentMissionIndex;
        emit lastCurrentIndexChanged(m_lastCurrentIndex);
    }
}

void MissionManager::removeAllWorker(MAV_MISSION_TYPE tp)
{
    mavlink_message_t message;
    m_bDown = false;
    emit progressPct(0);

    m_dedicatedLink = m_vehicle->priorityLink();
    mavlink_msg_mission_clear_all_pack_chan(qvgApp->toolbox()->mavlinkProtocol()->getSystemId(),
                                            qvgApp->toolbox()->mavlinkProtocol()->getComponentId(),
                                            m_dedicatedLink->getMavlinkChannel(),
                                            &message,
                                            m_vehicle->id(),
                                            MAV_COMP_ID_MISSIONPLANNER,
                                            tp);
    m_vehicle->sendMessageOnLink(m_vehicle->priorityLink(), message);
    _startAckTimeout(AckMissionClearAll);
}

void MissionManager::removeAll(void)
{
    if (inProgress())
        return;

    _clearAndDeleteMissionItems();
    m_currentMissionIndex = -1;
    m_lastCurrentIndex = -1;
    emit currentIndexChanged(-1);
    emit lastCurrentIndexChanged(-1);

    m_transactionInProgress = TransactionRemoveAll;
    m_retryCount = 0;
    emit inProgressChanged(true);
    m_typeMissionWrite = MAV_MISSION_TYPE_MISSION;
    removeAllWorker();
}

void MissionManager::generateResumeMission(int resumeIndex)
{
    if (inProgress())
        return;

    for (MissionItem *item: m_missionItems)
    {
        if (item->command() == MAV_CMD_DO_JUMP)
            return;
    }

    resumeIndex = qMin(resumeIndex, m_missionItems.count() - 1);

    int seqNum = 0;
    QList<MissionItem*> resumeMission;

    QList<MAV_CMD> includedResumeCommands;

    // If any command in this list occurs before the resumeIndex it will be added to the front of the mission
    includedResumeCommands << MAV_CMD_DO_CONTROL_VIDEO
                           << MAV_CMD_DO_SET_ROI
                           << MAV_CMD_DO_DIGICAM_CONFIGURE
                           << MAV_CMD_DO_DIGICAM_CONTROL
                           << MAV_CMD_DO_MOUNT_CONFIGURE
                           << MAV_CMD_DO_MOUNT_CONTROL
                           << MAV_CMD_DO_SET_CAM_TRIGG_DIST
                           << MAV_CMD_DO_FENCE_ENABLE
                           << MAV_CMD_IMAGE_START_CAPTURE
                           << MAV_CMD_IMAGE_STOP_CAPTURE
                           << MAV_CMD_VIDEO_START_CAPTURE
                           << MAV_CMD_VIDEO_STOP_CAPTURE
                           << MAV_CMD_DO_CHANGE_SPEED;

    bool addHomePosition = m_bSendHomePosition;
    int setCurrentIndex = addHomePosition ? 1 : 0;

    int resumeCommandCount = 0;
    for (int i=0; i<m_missionItems.count(); i++) {
        MissionItem* oldItem = m_missionItems[i];
        if ((i == 0 && addHomePosition) || i >= resumeIndex || includedResumeCommands.contains(oldItem->command())) {
            if (i < resumeIndex) {
                resumeCommandCount++;
            }
            MissionItem* newItem = new MissionItem(*oldItem, this);
            newItem->setIsCurrentItem( i == setCurrentIndex);
            newItem->setSequenceNumber(seqNum++);
            resumeMission.append(newItem);
        }
    }

    // De-dup and remove no-ops from the commands which were added to the front of the mission
    bool foundROI = false;
    bool foundCamTrigDist = false;
    QList<int> imageStartCameraIds;
    QList<int> imageStopCameraIds;
    QList<int> videoStartCameraIds;
    QList<int> videoStopCameraIds;
    while (resumeIndex >= 0) {
        MissionItem* resumeItem = resumeMission[resumeIndex];
        switch (resumeItem->command()) {
        case MAV_CMD_DO_SET_ROI:
            // Only keep the last one
            if (foundROI) {
                resumeMission.removeAt(resumeIndex);
            }
            foundROI = true;
            break;
        case MAV_CMD_DO_SET_CAM_TRIGG_DIST:
            // Only keep the last one
            if (foundCamTrigDist) {
                resumeMission.removeAt(resumeIndex);
            }
            foundCamTrigDist = true;
            break;
        case MAV_CMD_IMAGE_START_CAPTURE:
        {
            // FIXME: Handle single image capture
            int cameraId = resumeItem->param1().toDouble();

            if (resumeItem->param3() == 1) {
                // This is an individual image capture command, remove it
                resumeMission.removeAt(resumeIndex);
                break;
            }
            // If we already found an image stop, then all image start/stop commands are useless
            // De-dup repeated image start commands
            // Otherwise keep only the last image start
            if (imageStopCameraIds.contains(cameraId) || imageStartCameraIds.contains(cameraId)) {
                resumeMission.removeAt(resumeIndex);
            }
            if (!imageStopCameraIds.contains(cameraId)) {
                imageStopCameraIds.append(cameraId);
            }
        }
            break;
        case MAV_CMD_IMAGE_STOP_CAPTURE:
        {
            int cameraId = resumeItem->param1().toInt();
            // Image stop only matters to kill all previous image starts
            if (!imageStopCameraIds.contains(cameraId)) {
                imageStopCameraIds.append(cameraId);
            }
            resumeMission.removeAt(resumeIndex);
        }
            break;
        case MAV_CMD_VIDEO_START_CAPTURE:
        {
            int cameraId = resumeItem->param1().toInt();
            // If we've already found a video stop, then all video start/stop commands are useless
            // De-dup repeated video start commands
            // Otherwise keep only the last video start
            if (videoStopCameraIds.contains(cameraId) || videoStopCameraIds.contains(cameraId)) {
                resumeMission.removeAt(resumeIndex);
            }
            if (!videoStopCameraIds.contains(cameraId)) {
                videoStopCameraIds.append(cameraId);
            }
        }
            break;
        case MAV_CMD_VIDEO_STOP_CAPTURE:
        {
            int cameraId = resumeItem->param1().toInt();
            // Video stop only matters to kill all previous video starts
            if (!videoStopCameraIds.contains(cameraId)) {
                videoStopCameraIds.append(cameraId);
            }
            resumeMission.removeAt(resumeIndex);
        }
            break;
        default:
            break;
        }

        resumeIndex--;
    }

    // Send to vehicle
    _clearAndDeleteWriteMissionItems();
    for (int i=0; i<resumeMission.count(); i++) {
        m_writeMissionItems.append(new MissionItem(*resumeMission[i], this));
    }
    m_resumeMission = true;
    _writeMissionItemsWorker();

    // Clean up no longer needed resume items
    for (int i=0; i<resumeMission.count(); i++) {
        resumeMission[i]->deleteLater();
    }
}

bool MissionManager::isWriteIdle() const
{
    return m_writeMissionItems.count() == 0;
}

bool MissionManager::hasValidMissionItems() const
{
    return m_bHasValidMission;
}

VGVehicle * MissionManager::vehicle() const
{
    return m_vehicle;
}

bool MissionManager::IsMissonDown() const
{
    return m_bDown;
}

void MissionManager::_clearAndDeleteMissionItems(void)
{
    for (int i=0; i<m_missionItems.count(); i++) {
        m_missionItems[i]->deleteLater();
    }
    m_missionItems.clear();
}


void MissionManager::_clearAndDeleteWriteMissionItems(void)
{
    for (int i=0; i<m_writeMissionItems.count(); i++) {
        m_writeMissionItems[i]->deleteLater();
    }
    m_writeMissionItems.clear();
}
