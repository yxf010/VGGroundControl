#pragma once
// MESSAGE ASSIST_POSITION PACKING

#define MAVLINK_MSG_ID_ASSIST_POSITION 400

MAVPACKED(
typedef struct __mavlink_assist_position_t {
 int32_t start_latitude; /*< [degE7] start assist point, Latitude (WGS84), in degrees * 1E7*/
 int32_t start_longitude; /*< [degE7] start assist point, Longitude (WGS84, in degrees * 1E7*/
 int32_t start_altitude; /*< [mm] start assist point, Altitude (AMSL), in meters * 1000 (positive for up)*/
 int32_t end_latitude; /*< [degE7] end assist point, Latitude (WGS84), in degrees * 1E7*/
 int32_t end_longitude; /*< [degE7] end assist point, Longitude (WGS84, in degrees * 1E7*/
 int32_t end_altitude; /*< [mm] end assist point, Altitude (AMSL), in meters * 1000 (positive for up)*/
 int32_t int_latitude; /*< [degE7] interrupt point, Latitude (WGS84), in degrees * 1E7*/
 int32_t int_longitude; /*< [degE7] interrupt point, Longitude (WGS84, in degrees * 1E7*/
 int32_t int_altitude; /*< [mm] interrupt point, Altitude (AMSL), in meters * 1000 (positive for up)*/
 uint8_t assist_state; /*<  0x01 means start assist point valid, 0x02 means start assist point valid*/
}) mavlink_assist_position_t;

#define MAVLINK_MSG_ID_ASSIST_POSITION_LEN 37
#define MAVLINK_MSG_ID_ASSIST_POSITION_MIN_LEN 37
#define MAVLINK_MSG_ID_400_LEN 37
#define MAVLINK_MSG_ID_400_MIN_LEN 37

#define MAVLINK_MSG_ID_ASSIST_POSITION_CRC 147
#define MAVLINK_MSG_ID_400_CRC 147



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_ASSIST_POSITION { \
    400, \
    "ASSIST_POSITION", \
    10, \
    {  { "start_latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_assist_position_t, start_latitude) }, \
         { "start_longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_assist_position_t, start_longitude) }, \
         { "start_altitude", NULL, MAVLINK_TYPE_INT32_T, 0, 8, offsetof(mavlink_assist_position_t, start_altitude) }, \
         { "end_latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 12, offsetof(mavlink_assist_position_t, end_latitude) }, \
         { "end_longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 16, offsetof(mavlink_assist_position_t, end_longitude) }, \
         { "end_altitude", NULL, MAVLINK_TYPE_INT32_T, 0, 20, offsetof(mavlink_assist_position_t, end_altitude) }, \
         { "int_latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 24, offsetof(mavlink_assist_position_t, int_latitude) }, \
         { "int_longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 28, offsetof(mavlink_assist_position_t, int_longitude) }, \
         { "int_altitude", NULL, MAVLINK_TYPE_INT32_T, 0, 32, offsetof(mavlink_assist_position_t, int_altitude) }, \
         { "assist_state", NULL, MAVLINK_TYPE_UINT8_T, 0, 36, offsetof(mavlink_assist_position_t, assist_state) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_ASSIST_POSITION { \
    "ASSIST_POSITION", \
    10, \
    {  { "start_latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_assist_position_t, start_latitude) }, \
         { "start_longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_assist_position_t, start_longitude) }, \
         { "start_altitude", NULL, MAVLINK_TYPE_INT32_T, 0, 8, offsetof(mavlink_assist_position_t, start_altitude) }, \
         { "end_latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 12, offsetof(mavlink_assist_position_t, end_latitude) }, \
         { "end_longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 16, offsetof(mavlink_assist_position_t, end_longitude) }, \
         { "end_altitude", NULL, MAVLINK_TYPE_INT32_T, 0, 20, offsetof(mavlink_assist_position_t, end_altitude) }, \
         { "int_latitude", NULL, MAVLINK_TYPE_INT32_T, 0, 24, offsetof(mavlink_assist_position_t, int_latitude) }, \
         { "int_longitude", NULL, MAVLINK_TYPE_INT32_T, 0, 28, offsetof(mavlink_assist_position_t, int_longitude) }, \
         { "int_altitude", NULL, MAVLINK_TYPE_INT32_T, 0, 32, offsetof(mavlink_assist_position_t, int_altitude) }, \
         { "assist_state", NULL, MAVLINK_TYPE_UINT8_T, 0, 36, offsetof(mavlink_assist_position_t, assist_state) }, \
         } \
}
#endif

/**
 * @brief Pack a assist_position message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param start_latitude [degE7] start assist point, Latitude (WGS84), in degrees * 1E7
 * @param start_longitude [degE7] start assist point, Longitude (WGS84, in degrees * 1E7
 * @param start_altitude [mm] start assist point, Altitude (AMSL), in meters * 1000 (positive for up)
 * @param end_latitude [degE7] end assist point, Latitude (WGS84), in degrees * 1E7
 * @param end_longitude [degE7] end assist point, Longitude (WGS84, in degrees * 1E7
 * @param end_altitude [mm] end assist point, Altitude (AMSL), in meters * 1000 (positive for up)
 * @param int_latitude [degE7] interrupt point, Latitude (WGS84), in degrees * 1E7
 * @param int_longitude [degE7] interrupt point, Longitude (WGS84, in degrees * 1E7
 * @param int_altitude [mm] interrupt point, Altitude (AMSL), in meters * 1000 (positive for up)
 * @param assist_state  0x01 means start assist point valid, 0x02 means start assist point valid
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_assist_position_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               int32_t start_latitude, int32_t start_longitude, int32_t start_altitude, int32_t end_latitude, int32_t end_longitude, int32_t end_altitude, int32_t int_latitude, int32_t int_longitude, int32_t int_altitude, uint8_t assist_state)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_ASSIST_POSITION_LEN];
    _mav_put_int32_t(buf, 0, start_latitude);
    _mav_put_int32_t(buf, 4, start_longitude);
    _mav_put_int32_t(buf, 8, start_altitude);
    _mav_put_int32_t(buf, 12, end_latitude);
    _mav_put_int32_t(buf, 16, end_longitude);
    _mav_put_int32_t(buf, 20, end_altitude);
    _mav_put_int32_t(buf, 24, int_latitude);
    _mav_put_int32_t(buf, 28, int_longitude);
    _mav_put_int32_t(buf, 32, int_altitude);
    _mav_put_uint8_t(buf, 36, assist_state);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_ASSIST_POSITION_LEN);
#else
    mavlink_assist_position_t packet;
    packet.start_latitude = start_latitude;
    packet.start_longitude = start_longitude;
    packet.start_altitude = start_altitude;
    packet.end_latitude = end_latitude;
    packet.end_longitude = end_longitude;
    packet.end_altitude = end_altitude;
    packet.int_latitude = int_latitude;
    packet.int_longitude = int_longitude;
    packet.int_altitude = int_altitude;
    packet.assist_state = assist_state;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_ASSIST_POSITION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_ASSIST_POSITION;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_ASSIST_POSITION_MIN_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_CRC);
}

/**
 * @brief Pack a assist_position message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param start_latitude [degE7] start assist point, Latitude (WGS84), in degrees * 1E7
 * @param start_longitude [degE7] start assist point, Longitude (WGS84, in degrees * 1E7
 * @param start_altitude [mm] start assist point, Altitude (AMSL), in meters * 1000 (positive for up)
 * @param end_latitude [degE7] end assist point, Latitude (WGS84), in degrees * 1E7
 * @param end_longitude [degE7] end assist point, Longitude (WGS84, in degrees * 1E7
 * @param end_altitude [mm] end assist point, Altitude (AMSL), in meters * 1000 (positive for up)
 * @param int_latitude [degE7] interrupt point, Latitude (WGS84), in degrees * 1E7
 * @param int_longitude [degE7] interrupt point, Longitude (WGS84, in degrees * 1E7
 * @param int_altitude [mm] interrupt point, Altitude (AMSL), in meters * 1000 (positive for up)
 * @param assist_state  0x01 means start assist point valid, 0x02 means start assist point valid
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_assist_position_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   int32_t start_latitude,int32_t start_longitude,int32_t start_altitude,int32_t end_latitude,int32_t end_longitude,int32_t end_altitude,int32_t int_latitude,int32_t int_longitude,int32_t int_altitude,uint8_t assist_state)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_ASSIST_POSITION_LEN];
    _mav_put_int32_t(buf, 0, start_latitude);
    _mav_put_int32_t(buf, 4, start_longitude);
    _mav_put_int32_t(buf, 8, start_altitude);
    _mav_put_int32_t(buf, 12, end_latitude);
    _mav_put_int32_t(buf, 16, end_longitude);
    _mav_put_int32_t(buf, 20, end_altitude);
    _mav_put_int32_t(buf, 24, int_latitude);
    _mav_put_int32_t(buf, 28, int_longitude);
    _mav_put_int32_t(buf, 32, int_altitude);
    _mav_put_uint8_t(buf, 36, assist_state);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_ASSIST_POSITION_LEN);
#else
    mavlink_assist_position_t packet;
    packet.start_latitude = start_latitude;
    packet.start_longitude = start_longitude;
    packet.start_altitude = start_altitude;
    packet.end_latitude = end_latitude;
    packet.end_longitude = end_longitude;
    packet.end_altitude = end_altitude;
    packet.int_latitude = int_latitude;
    packet.int_longitude = int_longitude;
    packet.int_altitude = int_altitude;
    packet.assist_state = assist_state;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_ASSIST_POSITION_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_ASSIST_POSITION;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_ASSIST_POSITION_MIN_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_CRC);
}

/**
 * @brief Encode a assist_position struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param assist_position C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_assist_position_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_assist_position_t* assist_position)
{
    return mavlink_msg_assist_position_pack(system_id, component_id, msg, assist_position->start_latitude, assist_position->start_longitude, assist_position->start_altitude, assist_position->end_latitude, assist_position->end_longitude, assist_position->end_altitude, assist_position->int_latitude, assist_position->int_longitude, assist_position->int_altitude, assist_position->assist_state);
}

/**
 * @brief Encode a assist_position struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param assist_position C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_assist_position_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_assist_position_t* assist_position)
{
    return mavlink_msg_assist_position_pack_chan(system_id, component_id, chan, msg, assist_position->start_latitude, assist_position->start_longitude, assist_position->start_altitude, assist_position->end_latitude, assist_position->end_longitude, assist_position->end_altitude, assist_position->int_latitude, assist_position->int_longitude, assist_position->int_altitude, assist_position->assist_state);
}

/**
 * @brief Send a assist_position message
 * @param chan MAVLink channel to send the message
 *
 * @param start_latitude [degE7] start assist point, Latitude (WGS84), in degrees * 1E7
 * @param start_longitude [degE7] start assist point, Longitude (WGS84, in degrees * 1E7
 * @param start_altitude [mm] start assist point, Altitude (AMSL), in meters * 1000 (positive for up)
 * @param end_latitude [degE7] end assist point, Latitude (WGS84), in degrees * 1E7
 * @param end_longitude [degE7] end assist point, Longitude (WGS84, in degrees * 1E7
 * @param end_altitude [mm] end assist point, Altitude (AMSL), in meters * 1000 (positive for up)
 * @param int_latitude [degE7] interrupt point, Latitude (WGS84), in degrees * 1E7
 * @param int_longitude [degE7] interrupt point, Longitude (WGS84, in degrees * 1E7
 * @param int_altitude [mm] interrupt point, Altitude (AMSL), in meters * 1000 (positive for up)
 * @param assist_state  0x01 means start assist point valid, 0x02 means start assist point valid
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_assist_position_send(mavlink_channel_t chan, int32_t start_latitude, int32_t start_longitude, int32_t start_altitude, int32_t end_latitude, int32_t end_longitude, int32_t end_altitude, int32_t int_latitude, int32_t int_longitude, int32_t int_altitude, uint8_t assist_state)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_ASSIST_POSITION_LEN];
    _mav_put_int32_t(buf, 0, start_latitude);
    _mav_put_int32_t(buf, 4, start_longitude);
    _mav_put_int32_t(buf, 8, start_altitude);
    _mav_put_int32_t(buf, 12, end_latitude);
    _mav_put_int32_t(buf, 16, end_longitude);
    _mav_put_int32_t(buf, 20, end_altitude);
    _mav_put_int32_t(buf, 24, int_latitude);
    _mav_put_int32_t(buf, 28, int_longitude);
    _mav_put_int32_t(buf, 32, int_altitude);
    _mav_put_uint8_t(buf, 36, assist_state);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_ASSIST_POSITION, buf, MAVLINK_MSG_ID_ASSIST_POSITION_MIN_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_CRC);
#else
    mavlink_assist_position_t packet;
    packet.start_latitude = start_latitude;
    packet.start_longitude = start_longitude;
    packet.start_altitude = start_altitude;
    packet.end_latitude = end_latitude;
    packet.end_longitude = end_longitude;
    packet.end_altitude = end_altitude;
    packet.int_latitude = int_latitude;
    packet.int_longitude = int_longitude;
    packet.int_altitude = int_altitude;
    packet.assist_state = assist_state;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_ASSIST_POSITION, (const char *)&packet, MAVLINK_MSG_ID_ASSIST_POSITION_MIN_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_CRC);
#endif
}

/**
 * @brief Send a assist_position message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_assist_position_send_struct(mavlink_channel_t chan, const mavlink_assist_position_t* assist_position)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_assist_position_send(chan, assist_position->start_latitude, assist_position->start_longitude, assist_position->start_altitude, assist_position->end_latitude, assist_position->end_longitude, assist_position->end_altitude, assist_position->int_latitude, assist_position->int_longitude, assist_position->int_altitude, assist_position->assist_state);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_ASSIST_POSITION, (const char *)assist_position, MAVLINK_MSG_ID_ASSIST_POSITION_MIN_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_CRC);
#endif
}

#if MAVLINK_MSG_ID_ASSIST_POSITION_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_assist_position_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  int32_t start_latitude, int32_t start_longitude, int32_t start_altitude, int32_t end_latitude, int32_t end_longitude, int32_t end_altitude, int32_t int_latitude, int32_t int_longitude, int32_t int_altitude, uint8_t assist_state)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_int32_t(buf, 0, start_latitude);
    _mav_put_int32_t(buf, 4, start_longitude);
    _mav_put_int32_t(buf, 8, start_altitude);
    _mav_put_int32_t(buf, 12, end_latitude);
    _mav_put_int32_t(buf, 16, end_longitude);
    _mav_put_int32_t(buf, 20, end_altitude);
    _mav_put_int32_t(buf, 24, int_latitude);
    _mav_put_int32_t(buf, 28, int_longitude);
    _mav_put_int32_t(buf, 32, int_altitude);
    _mav_put_uint8_t(buf, 36, assist_state);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_ASSIST_POSITION, buf, MAVLINK_MSG_ID_ASSIST_POSITION_MIN_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_CRC);
#else
    mavlink_assist_position_t *packet = (mavlink_assist_position_t *)msgbuf;
    packet->start_latitude = start_latitude;
    packet->start_longitude = start_longitude;
    packet->start_altitude = start_altitude;
    packet->end_latitude = end_latitude;
    packet->end_longitude = end_longitude;
    packet->end_altitude = end_altitude;
    packet->int_latitude = int_latitude;
    packet->int_longitude = int_longitude;
    packet->int_altitude = int_altitude;
    packet->assist_state = assist_state;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_ASSIST_POSITION, (const char *)packet, MAVLINK_MSG_ID_ASSIST_POSITION_MIN_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_LEN, MAVLINK_MSG_ID_ASSIST_POSITION_CRC);
#endif
}
#endif

#endif

// MESSAGE ASSIST_POSITION UNPACKING


/**
 * @brief Get field start_latitude from assist_position message
 *
 * @return [degE7] start assist point, Latitude (WGS84), in degrees * 1E7
 */
static inline int32_t mavlink_msg_assist_position_get_start_latitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  0);
}

/**
 * @brief Get field start_longitude from assist_position message
 *
 * @return [degE7] start assist point, Longitude (WGS84, in degrees * 1E7
 */
static inline int32_t mavlink_msg_assist_position_get_start_longitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  4);
}

/**
 * @brief Get field start_altitude from assist_position message
 *
 * @return [mm] start assist point, Altitude (AMSL), in meters * 1000 (positive for up)
 */
static inline int32_t mavlink_msg_assist_position_get_start_altitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  8);
}

/**
 * @brief Get field end_latitude from assist_position message
 *
 * @return [degE7] end assist point, Latitude (WGS84), in degrees * 1E7
 */
static inline int32_t mavlink_msg_assist_position_get_end_latitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  12);
}

/**
 * @brief Get field end_longitude from assist_position message
 *
 * @return [degE7] end assist point, Longitude (WGS84, in degrees * 1E7
 */
static inline int32_t mavlink_msg_assist_position_get_end_longitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  16);
}

/**
 * @brief Get field end_altitude from assist_position message
 *
 * @return [mm] end assist point, Altitude (AMSL), in meters * 1000 (positive for up)
 */
static inline int32_t mavlink_msg_assist_position_get_end_altitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  20);
}

/**
 * @brief Get field int_latitude from assist_position message
 *
 * @return [degE7] interrupt point, Latitude (WGS84), in degrees * 1E7
 */
static inline int32_t mavlink_msg_assist_position_get_int_latitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  24);
}

/**
 * @brief Get field int_longitude from assist_position message
 *
 * @return [degE7] interrupt point, Longitude (WGS84, in degrees * 1E7
 */
static inline int32_t mavlink_msg_assist_position_get_int_longitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  28);
}

/**
 * @brief Get field int_altitude from assist_position message
 *
 * @return [mm] interrupt point, Altitude (AMSL), in meters * 1000 (positive for up)
 */
static inline int32_t mavlink_msg_assist_position_get_int_altitude(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  32);
}

/**
 * @brief Get field assist_state from assist_position message
 *
 * @return  0x01 means start assist point valid, 0x02 means start assist point valid
 */
static inline uint8_t mavlink_msg_assist_position_get_assist_state(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  36);
}

/**
 * @brief Decode a assist_position message into a struct
 *
 * @param msg The message to decode
 * @param assist_position C-struct to decode the message contents into
 */
static inline void mavlink_msg_assist_position_decode(const mavlink_message_t* msg, mavlink_assist_position_t* assist_position)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    assist_position->start_latitude = mavlink_msg_assist_position_get_start_latitude(msg);
    assist_position->start_longitude = mavlink_msg_assist_position_get_start_longitude(msg);
    assist_position->start_altitude = mavlink_msg_assist_position_get_start_altitude(msg);
    assist_position->end_latitude = mavlink_msg_assist_position_get_end_latitude(msg);
    assist_position->end_longitude = mavlink_msg_assist_position_get_end_longitude(msg);
    assist_position->end_altitude = mavlink_msg_assist_position_get_end_altitude(msg);
    assist_position->int_latitude = mavlink_msg_assist_position_get_int_latitude(msg);
    assist_position->int_longitude = mavlink_msg_assist_position_get_int_longitude(msg);
    assist_position->int_altitude = mavlink_msg_assist_position_get_int_altitude(msg);
    assist_position->assist_state = mavlink_msg_assist_position_get_assist_state(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_ASSIST_POSITION_LEN? msg->len : MAVLINK_MSG_ID_ASSIST_POSITION_LEN;
        memset(assist_position, 0, MAVLINK_MSG_ID_ASSIST_POSITION_LEN);
    memcpy(assist_position, _MAV_PAYLOAD(msg), len);
#endif
}
