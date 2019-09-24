#ifndef __VGMAVLINKCODE_H__
#define __VGMAVLINKCODE_H__

#include "mavlink_types.h"
#include "mavlink.h"
#include <QString>
#include <QPointF>

#define MissionMod      "Mission"
#define DenyMod         "deny"
#define ManualMod       "Manual"
#define HoldMod         "Hold"
#define ReturnMod		"Return"
#define MagMission		"MagMsm"
#define Landing		    "Landing"

class MAVLinkProtocol;
class MissionItem;
class QVariant;
class QGeoCoordinate;

namespace VGMavLinkCode {
    uint32_t GetFlightModeByString(const QString &mode); 
    QString GetFlightModeName(uint32_t mode);
    QStringList SurpportModes();

    bool InitMavMessage(mavlink_message_t &msg, uint8_t id, const void *payload, int len);
    void EncodeRTCM(mavlink_message_t &msg, const QByteArray arr, int flag=1, const MAVLinkProtocol *p = NULL, int ch = 0, int sysId = 0);
    void EncodeHartbeat(mavlink_message_t &msg, int cpnt=0, int sysId = 0, const mavlink_heartbeat_t *h=NULL);
    void EncodeCommands(mavlink_message_t &msg, int cpnt, MAV_CMD cmd, const MAVLinkProtocol *p = NULL, int ch = 0, int sysId=0,
        float p1 = 0, float p2 = 0, float p3 = 0, float p4 = 0, float p5 = 0, float p6 = 0, float p7 = 0 );
    void EncodeCommandInt(mavlink_message_t &msg, int cpnt, MAV_CMD cmd, const MAVLinkProtocol *p = NULL, int ch = 0, int sysId=0,
        float p1 = 0, float p2 = 0, float p3 = 0, float p4 = 0, int p5 = 0, int p6 = 0, float p7 = 0);
    void EncodeMissionCount(mavlink_message_t &msg, uint32_t count, MAV_MISSION_TYPE tp=MAV_MISSION_TYPE_MISSION, MAVLinkProtocol *p = NULL, int ch = 0, int cmp=1, int sysId = 1);
    void EncodeMissionItem(mavlink_message_t &msg, const MissionItem &item, bool cur, MAVLinkProtocol *p = NULL, int ch = 0, int sysId = 1);
    void EncodeMissionItemInt(mavlink_message_t &msg, const MissionItem &item, bool cur, const MAVLinkProtocol *p = NULL, int ch = 0, int sysId = 1);
    bool EncodeSetParameter(mavlink_message_t &msg, const QString &key, const QVariant &v, const MAVLinkProtocol *p = NULL, int ch = 0, int sysId = 0);
    bool EncodeGetParameter(mavlink_message_t &msg, const QString &key, const MAVLinkProtocol *p = NULL, int c = 0, int sysId = 0);
    bool EncodeSetMode(mavlink_message_t &msg, const QString &mod, uint8_t baseMod = 0xFC, const MAVLinkProtocol *p = NULL, int ch = 0, int sysId = 0);
    void EncodeReadItem(mavlink_message_t &msg, int seq, bool bInt, MAV_MISSION_TYPE tp=MAV_MISSION_TYPE_MISSION, MAVLinkProtocol *p = NULL, int ch = 0, int sysId = 0);
    void EncodeQXAccount(mavlink_message_t &msg, const QString &acc, const QString &pswd, MAVLinkProtocol *p = NULL, int ch = 0);

    QString DecodeParameter(const mavlink_message_t &msg, QVariant &v);
    int DecodeMissionRequest(const mavlink_message_t &msg);
    bool DecodeMissionItem(const mavlink_message_t &msg, MissionItem &item);
    bool DecodeCommandAck(const mavlink_message_t &msg, uint16_t &command, MAV_RESULT &result);
    bool DecodeHomePos(const mavlink_message_t &msg, QGeoCoordinate &home);
    int DecodeSupport(const mavlink_message_t &msg, QGeoCoordinate &cEenter, QGeoCoordinate &cTreturn, QGeoCoordinate &cCtn);
    bool DecodeGlobalPos(const mavlink_message_t &msg, QGeoCoordinate &pos, double &vHor, double &vVer, double &alt);
    bool DecodeGpsSatlate(const mavlink_message_t &msg, QGeoCoordinate &pos, int &nNumb, uint8_t &fix, double &pre, uint16_t& cnt);
    bool DecodeSpray(const mavlink_message_t &msg, double &speed, double &volumeSprayed, uint8_t &stat, uint8_t &mode);
    QString DecodeQXAccount(const mavlink_message_t &msg, QString &pswd);
}

#endif // VGGLOBALFUN
