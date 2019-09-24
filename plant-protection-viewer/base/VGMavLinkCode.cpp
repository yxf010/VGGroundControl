#include "VGMavLinkCode.h"
#include "MAVLinkProtocol.h"
#include "MissionItem.h"
#include "VGGlobalFun.h"
#include "px4_custom_mode.h"
#include <QGeoCoordinate>

static QMap<QString, px4_custom_mode> &GetModeMap()
{
    static QMap<QString, px4_custom_mode> s_Map;
    if (s_Map.size() == 0)
    {
        px4_custom_mode mode;
        mode.data = 0;
        mode.main_mode = PX4_CUSTOM_MAIN_MODE_AUTO;
        mode.sub_mode = PX4_CUSTOM_SUB_MODE_AUTO_MISSION;
        s_Map[MissionMod] = mode;

        mode.data = 0;
        mode.main_mode = PX4_CUSTOM_MAIN_MODE_AUTO;
        mode.sub_mode = PX4_CUSTOM_SUB_MODE_AUTO_LOITER;
        s_Map[HoldMod] = mode;

        mode.data = 0;
        mode.main_mode = PX4_CUSTOM_MAIN_MODE_MANUAL;
        mode.sub_mode = 0;
        s_Map[ManualMod] = mode;

		mode.data = 0;
		mode.main_mode = PX4_CUSTOM_MAIN_MODE_AUTO;
		mode.sub_mode = PX4_CUSTOM_SUB_MODE_AUTO_RTL;
        s_Map[ReturnMod] = mode;

        mode.data = 0;
        mode.main_mode = PX4_CUSTOM_MAIN_MODE_AUTO;
        mode.sub_mode = PX4_CUSTOM_SUB_MODE_AUTO_MAG_MISSION;
        s_Map[MagMission] = mode;

        mode.data = 0;
        mode.main_mode = PX4_CUSTOM_MAIN_MODE_AUTO;
        mode.sub_mode = PX4_CUSTOM_SUB_MODE_AUTO_LAND;
        s_Map[Landing] = mode;
    }
    return s_Map;
}

uint32_t VGMavLinkCode::GetFlightModeByString(const QString &mode)
{
        QMap<QString, px4_custom_mode> &modes = GetModeMap();
        QMap<QString, px4_custom_mode>::const_iterator itr = modes.find(mode);
        if (itr != modes.end())
            return itr.value().data;

        return 0;
}

QString VGMavLinkCode::GetFlightModeName(uint32_t mode)
{
    QMap<QString, px4_custom_mode> &modes = GetModeMap();
    QMap<QString, px4_custom_mode>::const_iterator itr = modes.begin();
    for (; itr != modes.end(); ++itr)
    {
        if (itr.value().data == mode)
            return itr.key();
    }
    return QString();
}

QStringList VGMavLinkCode::SurpportModes()
{
    return GetModeMap().keys();
}

bool VGMavLinkCode::InitMavMessage(mavlink_message_t &msg, uint8_t id, const void *payload, int len)
{
    if (payload && len > 0)
    {
        memset(&msg, 0, sizeof(mavlink_message_t));
        msg.msgid = id;
        memcpy(msg.payload64, payload, len);
        msg.len = len;
        return true;
    }
    return false;
}

void VGMavLinkCode::EncodeRTCM(mavlink_message_t &msg, const QByteArray arr, int flag, const MAVLinkProtocol *p, int ch, int)
{
    mavlink_gps_rtcm_data_t packet_in;
    packet_in.flags = flag;
    packet_in.len = arr.size();
    memcpy(packet_in.data, arr.data(), arr.size());
    mavlink_msg_gps_rtcm_data_encode_chan(p ? p->getSystemId() : 0,
        p ? p->getComponentId() : 0
        ,ch , &msg, &packet_in);
}

void VGMavLinkCode::EncodeHartbeat(mavlink_message_t &msg, int cpnt, int sysId, const mavlink_heartbeat_t *h)
{
    if (!h)
    {
        mavlink_heartbeat_t state = { 0 };
        mavlink_msg_heartbeat_encode(sysId, cpnt, &msg, &state);
        return;
    }
    mavlink_msg_heartbeat_encode(sysId, cpnt, &msg, h);
}

void VGMavLinkCode::EncodeCommands(mavlink_message_t &msg, int cpnt, MAV_CMD cmd, const MAVLinkProtocol *p,
    int ch /*= 0*/, int sysId, float p1 /*= 0*/, float p2 /*= 0*/, float p3 /*= 0*/,
    float p4 /*= 0*/, float p5 /*= 0*/, float p6 /*= 0*/, float p7 /*= 0*/ )
{
    mavlink_command_long_t  cmdMav;
    memset(&cmdMav, 0, sizeof(cmdMav));

    cmdMav.command = cmd;
    cmdMav.confirmation = 0;
    cmdMav.param1 = p1;
    cmdMav.param2 = p2;
    cmdMav.param3 = p3;
    cmdMav.param4 = p4;
    cmdMav.param5 = p5;
    cmdMav.param6 = p6;
    cmdMav.param7 = p7;
    cmdMav.target_system = sysId;
    cmdMav.target_component = cpnt;
    mavlink_msg_command_long_encode_chan(p?p->getSystemId():0,
        p ? p->getComponentId():0,
        ch, &msg, &cmdMav);
}

void VGMavLinkCode::EncodeCommandInt(mavlink_message_t &msg, int cpnt, MAV_CMD cmd, const MAVLinkProtocol *p,
    int ch, int sysId, float p1, float p2, float p3,
    float p4, int p5, int p6, float p7)
{
    mavlink_command_int_t  cmdMav = {};
    memset(&cmdMav, 0, sizeof(cmdMav));

    cmdMav.command = cmd;
    cmdMav.current = false;
    cmdMav.autocontinue = true;
    cmdMav.param1 = p1;
    cmdMav.param2 = p2;
    cmdMav.param3 = p3;
    cmdMav.param4 = p4;
    cmdMav.x = p5;
    cmdMav.y = p6;
    cmdMav.z = p7;
    cmdMav.target_system = sysId;
    cmdMav.target_component = cpnt;
    mavlink_msg_command_int_encode_chan(p ? p->getSystemId() : 0,
        p ? p->getComponentId() : 0,
        ch, &msg, &cmdMav);
}

void VGMavLinkCode::EncodeMissionCount(mavlink_message_t &msg, uint32_t count, MAV_MISSION_TYPE tp, MAVLinkProtocol *p, int ch, int cmp, int sysId)
{
    uint8_t locSys = p ? p->getSystemId() : 0;
    uint8_t locCmp = p ? p->getComponentId() : 0;
    mavlink_msg_mission_count_pack_chan(locSys, locCmp, ch, &msg, sysId, cmp, count, tp);
}

void VGMavLinkCode::EncodeMissionItem(mavlink_message_t &msg, const MissionItem &item, bool cur, MAVLinkProtocol *p, int ch, int sysId)
{
    mavlink_mission_item_t missionItem;
    memset(&missionItem, 0, sizeof(missionItem));
	
    double x = item.param5().toDouble();
    double y = item.param6().toDouble();
    ShareFunction::toGps(x, y);

    missionItem.target_system = sysId;
    missionItem.target_component = MAV_COMP_ID_MISSIONPLANNER;
    missionItem.seq = item.sequenceNumber();
    missionItem.command = item.command();
    missionItem.param1 = item.param1().toDouble();
    missionItem.param2 = item.param2().toDouble();
    missionItem.param3 = item.param3().toDouble();
    missionItem.param4 = item.param4().toDouble();

    missionItem.x = x;
    missionItem.y = y;
    missionItem.z = item.param7().toDouble();
    missionItem.frame = item.frame();
    missionItem.current = cur;
    missionItem.autocontinue = item.autoContinue();
    missionItem.mission_type = item.missionType();

    mavlink_msg_mission_item_encode_chan(p ? p->getSystemId() : 0,
        p ? p->getComponentId() : 0,
        ch,
        &msg,
        &missionItem);
}

void VGMavLinkCode::EncodeMissionItemInt(mavlink_message_t &msg, const MissionItem &item, bool cur, const MAVLinkProtocol *p, int ch, int sysId)
{
    mavlink_mission_item_int_t missionItem;
    memset(&missionItem, 0, sizeof(missionItem));

    double x = item.param5().toDouble();
    double y = item.param6().toDouble();
    ShareFunction::toGps(x, y);

    missionItem.target_system = sysId;
    missionItem.target_component = MAV_COMP_ID_MISSIONPLANNER;
    missionItem.seq = item.sequenceNumber();
    missionItem.command = item.command();
    missionItem.param1 = item.param1().toDouble();
    missionItem.param2 = item.param2().toDouble();
    missionItem.param3 = item.param3().toDouble();
    missionItem.param4 = item.param4().toDouble();

    missionItem.x = x * 1e7;
    missionItem.y = y  * 1e7;
    missionItem.z = item.param7().toDouble();
    missionItem.frame = item.frame();
    missionItem.current = cur;
    missionItem.autocontinue = item.autoContinue();
    missionItem.mission_type = item.missionType();

    mavlink_msg_mission_item_int_encode_chan(p ? p->getSystemId() : 0,
        p ? p->getComponentId() : 0,
        ch,
        &msg,
        &missionItem);
}

bool VGMavLinkCode::EncodeSetParameter(mavlink_message_t &msg, const QString &key, const QVariant &v, const MAVLinkProtocol *p, int ch, int sysId)
{
    if (key.isEmpty() || v.isNull())
        return false;

    MAV_PARAM_TYPE type;
    float pTmp;
    switch (v.type())
    {
    case QVariant::Bool:
        type = MAV_PARAM_TYPE_UINT8;
        pTmp = v.toBool();
        break;
    case QVariant::Char:
        type = MAV_PARAM_TYPE_INT8;
        *(QChar*)&pTmp = v.toChar();
        break;
    case QVariant::UInt:
        type = MAV_PARAM_TYPE_UINT32;
        *(unsigned*)&pTmp = v.toUInt();
        break;
    case QVariant::Int:
        type = MAV_PARAM_TYPE_INT32;
        *(int*)&pTmp = v.toInt();
        break;
    case QVariant::Double:
    case QMetaType::Float:
        type = MAV_PARAM_TYPE_REAL32;
        pTmp = v.toDouble();
        break;
    default:
        return false;
    }
    mavlink_msg_param_set_pack_chan(p ? p->getSystemId() : 0,
        p ? p->getComponentId() : 0,
        ch, &msg, sysId, BASE::defaultComponentId,
        key.toUtf8().data(), pTmp, type);

    return true;
}

bool VGMavLinkCode::EncodeGetParameter(mavlink_message_t &msg, const QString &key, const MAVLinkProtocol *p, int c, int sysId)
{
    if (key.isEmpty())
        return false;
    mavlink_msg_param_request_read_pack(p?p->getSystemId():0,		// Our system id
        p?p->getComponentId():0,	                                // Our component id
        &msg, sysId, c,
        key.toUtf8().data(), -1);

    return true;
}

bool VGMavLinkCode::EncodeSetMode(mavlink_message_t &msg, const QString &mod, uint8_t baseMod, const MAVLinkProtocol *p, int ch, int sysId)
{
    if (uint32_t cstmMode = VGMavLinkCode::GetFlightModeByString(mod))
    {
        // setFlightMode will only set MAV_MODE_FLAG_CUSTOM_MODE_ENABLED in base_mode, we need to move back in the existing
        uint8_t baseMode = baseMod & ~MAV_MODE_FLAG_DECODE_POSITION_CUSTOM_MODE;
        baseMode |= MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;

        mavlink_msg_set_mode_pack_chan(p ? p->getSystemId() : 0,
            p ? p->getComponentId() : 0,
            ch, &msg, sysId,
            baseMode, cstmMode);
        return true;
    }
    return false;
}

void VGMavLinkCode::EncodeReadItem(mavlink_message_t &msg, int seq, bool bInt, MAV_MISSION_TYPE tp, MAVLinkProtocol *p, int ch, int sysId)
{
    if (bInt)
    {
        mavlink_mission_request_int_t missionRequest;
        missionRequest.target_system = sysId;
        missionRequest.target_component = MAV_COMP_ID_MISSIONPLANNER;
        missionRequest.seq = seq;
        missionRequest.mission_type = tp;

        mavlink_msg_mission_request_int_encode_chan(p?p->getSystemId():0,
            p ? p->getComponentId():0,
            ch,
            &msg,
            &missionRequest);
    }
    else
    {
        mavlink_mission_request_t missionRequest;
        missionRequest.target_system = sysId;
        missionRequest.target_component = MAV_COMP_ID_MISSIONPLANNER;
        missionRequest.seq = seq;
        missionRequest.mission_type = tp;

        mavlink_msg_mission_request_encode_chan(p ? p->getSystemId() : 0,
            p ? p->getComponentId() : 0,
            ch,
            &msg,
            &missionRequest);
    }
}

void VGMavLinkCode::EncodeQXAccount(mavlink_message_t &msg, const QString &acc, const QString &pswd, MAVLinkProtocol *p /*= NULL*/, int ch /*= 0*/)
{
    mavlink_msg_qx_account_pack_chan(p ? p->getSystemId() : 0,
        p ? p->getComponentId() : 0, ch, &msg, (uint8_t*)acc.toUtf8().data()
        , (uint8_t*)pswd.toUtf8().data(), 0);
}

QString VGMavLinkCode::DecodeParameter(const mavlink_message_t &msg, QVariant &v)
{
    if (msg.msgid != MAVLINK_MSG_ID_PARAM_VALUE)
        return QString();

    mavlink_param_value_t rawValue;
    mavlink_msg_param_value_decode(&msg, &rawValue);
    mavlink_param_union_t paramVal;
    paramVal.param_float = rawValue.param_value;
    paramVal.type = rawValue.param_type;

    switch (rawValue.param_type)
    {
    case MAV_PARAM_TYPE_REAL32:
        v.setValue<float>(paramVal.param_float);
        break;
    case MAV_PARAM_TYPE_UINT8:
        v.setValue<uint8_t>(paramVal.param_uint8);
        break;
    case MAV_PARAM_TYPE_INT8:
        v.setValue<int8_t>(paramVal.param_int8);
        break;
    case MAV_PARAM_TYPE_UINT16:
        v.setValue<uint16_t>(paramVal.param_uint16);
        break;
    case MAV_PARAM_TYPE_INT16:
        v.setValue<int16_t>(paramVal.param_int16);
        break;
    case MAV_PARAM_TYPE_UINT32:
        v.setValue<unsigned>(paramVal.param_uint32);
        break;
    case MAV_PARAM_TYPE_INT32:
        v.setValue<int>(paramVal.param_int32);
        break;
    default:
        v = QVariant();
    }

    QByteArray bytes(rawValue.param_id, MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN);
    return QString::fromUtf8(bytes);
}

int VGMavLinkCode::DecodeMissionRequest(const mavlink_message_t &msg)
{
    if (msg.msgid == MAVLINK_MSG_ID_MISSION_REQUEST_INT || msg.msgid == MAVLINK_MSG_ID_MISSION_REQUEST)
    {
        mavlink_mission_request_t missionRequest;
        mavlink_msg_mission_request_decode(&msg, &missionRequest);
        return missionRequest.seq;
    }
    return -1;
}

bool VGMavLinkCode::DecodeMissionItem(const mavlink_message_t &msg, MissionItem &item)
{
    if (msg.msgid == MAVLINK_MSG_ID_MISSION_ITEM_INT)
    {
        mavlink_mission_item_int_t missionItem;
        mavlink_msg_mission_item_int_decode(&msg, &missionItem);

        item.setCommand((MAV_CMD)missionItem.command);
        item.setFrame((MAV_FRAME)missionItem.frame);
        item.setParam1(missionItem.param1);
        item.setParam2(missionItem.param2);
        item.setParam3(missionItem.param3 == 1.0);
        item.setParam4(missionItem.param4);
        item.setParam5(double(missionItem.x) / 1e7);
        item.setParam6(double(missionItem.y) / 1e7);
        item.setParam7(double(missionItem.z));
        item.setAutoContinue(missionItem.autocontinue);
        item.setIsCurrentItem(missionItem.current);
        item.setSequenceNumber(missionItem.seq);
        return true;
    }
    else if(msg.msgid == MAVLINK_MSG_ID_MISSION_ITEM)
    {
        mavlink_mission_item_t missionItem;
        mavlink_msg_mission_item_decode(&msg, &missionItem);

        item.setCommand((MAV_CMD)missionItem.command);
        item.setFrame((MAV_FRAME)missionItem.frame);
        item.setParam1(missionItem.param1);
        item.setParam2(missionItem.param2);
        item.setParam3(missionItem.param3 == 1.0);
        item.setParam4(missionItem.param4);
        item.setParam5(double(missionItem.x));
        item.setParam6(double(missionItem.y));
        item.setParam7(double(missionItem.z));
        item.setAutoContinue(missionItem.autocontinue);
        item.setIsCurrentItem(missionItem.current);
        item.setSequenceNumber(missionItem.seq);
        return true;
    }
    return false;
}

bool VGMavLinkCode::DecodeCommandAck(const mavlink_message_t &msg, uint16_t &command, MAV_RESULT &result)
{
    if (msg.msgid == MAVLINK_MSG_ID_COMMAND_ACK)
    {
        mavlink_command_ack_t ack;
        mavlink_msg_command_ack_decode(&msg, &ack);
        command = ack.command;
        result = MAV_RESULT(ack.result);
        return true;
    }
    return false;
}

bool VGMavLinkCode::DecodeHomePos(const mavlink_message_t &msg, QGeoCoordinate &home)
{
    if (msg.msgid != MAVLINK_MSG_ID_HOME_POSITION)
        return false;

    mavlink_home_position_t homePos;
    mavlink_msg_home_position_decode(&msg, &homePos);

    double lat = homePos.latitude * 1e-7;
    double lon = homePos.longitude * 1e-7;
    double alt = homePos.altitude * 1e-3;
    home = QGeoCoordinate(lat, lon, alt);

    return true;
}

int VGMavLinkCode::DecodeSupport(const mavlink_message_t &msg, QGeoCoordinate &cE, QGeoCoordinate &cR, QGeoCoordinate &cCtn)
{
    if (MAVLINK_MSG_ID_ASSIST_POSITION != msg.msgid)
        return -1;

    mavlink_assist_position_t supports;
    mavlink_msg_assist_position_decode(&msg, &supports);
    double lat = supports.start_latitude*1e-7;
    double lon = supports.start_longitude*1e-7;
    double alt = supports.start_altitude*1e-3;
    cE = QGeoCoordinate(lat, lon, alt);

    lat = supports.end_latitude*1e-7;
    lon = supports.end_longitude*1e-7;
    alt = supports.end_altitude*1e-3;
    cR = QGeoCoordinate(lat, lon, alt);

    lat = supports.int_latitude*1e-7;
    lon = supports.int_longitude*1e-7;
    alt = supports.int_altitude*1e-3;
    cCtn = QGeoCoordinate(lat, lon, alt);
    return supports.assist_state;
}

bool VGMavLinkCode::DecodeGlobalPos(const mavlink_message_t &msg, QGeoCoordinate &posG, double &vHor, double &vVer, double &alt)
{
    if (MAVLINK_MSG_ID_GLOBAL_POSITION_INT != msg.msgid)
        return false;

    mavlink_global_position_int_t pos;
    mavlink_msg_global_position_int_decode(&msg, &pos);

    posG.setLatitude(double(pos.lat) / 1E7);
    posG.setLongitude(double(pos.lon) / 1E7);
    posG.setAltitude(pos.relative_alt / 1000.0);
    alt = pos.alt / 1000.0;

    double speedX = pos.vx / 100.0;
    double speedY = pos.vy / 100.0;
    vVer = pos.vz / 100.0;
    vHor = qSqrt(speedX*speedX + speedY*speedY);
    return true;
}

bool VGMavLinkCode::DecodeGpsSatlate(const mavlink_message_t &msg, QGeoCoordinate &pos, int &nNumb, uint8_t &fix, double &preH, uint16_t& cnt)
{
    if (MAVLINK_MSG_ID_GPS_RAW_INT != msg.msgid)
        return false;

    mavlink_gps_raw_int_t gpsRawInt;
    mavlink_msg_gps_raw_int_decode(&msg, &gpsRawInt);
    fix = gpsRawInt.fix_type;
    pos.setLatitude(double(gpsRawInt.lat) / 1E7);
    pos.setLongitude(double(gpsRawInt.lon) / 1E7);
    pos.setAltitude(gpsRawInt.alt / 1000.0);
    nNumb = gpsRawInt.satellites_visible;
    preH = double(gpsRawInt.h_acc)/1000;
	cnt = gpsRawInt.cog;
    return true;
}

bool VGMavLinkCode::DecodeSpray(const mavlink_message_t &msg, double &speed, double &volumeSprayed, uint8_t &stat, uint8_t &mode)
{
    if (MAVLINK_MSG_ID_SPRAY_VALUE != msg.msgid)
        return false;

    mavlink_spray_value_t spray;
    mavlink_msg_spray_value_decode(&msg, &spray);
    speed = spray.spray_speed;
    volumeSprayed = spray.volume_sprayed;
    stat = spray.spray_state;
    mode = spray.spray_mode;
    return true;
}

QString VGMavLinkCode::DecodeQXAccount(const mavlink_message_t &msg, QString &pswd)
{
    if (msg.msgid != MAVLINK_MSG_ID_QX_ACCOUNT)
        return QString();

    mavlink_qx_account_t qxacc = { 0 };
    mavlink_msg_qx_account_decode(&msg, &qxacc);
    pswd = (char*)qxacc.pswd;
    return QString((char*)qxacc.acc);
}
