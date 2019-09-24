#pragma once
// MESSAGE SPRAY_VALUE PACKING

#define MAVLINK_MSG_ID_SPRAY_VALUE 220

MAVPACKED(
typedef struct __mavlink_spray_value_t {
 float spray_speed; /*< [ml/s] spray speed*/
 float volume_sprayed; /*< [ml] volume sprayed*/
 uint8_t spray_state; /*<  nowater state*/
 uint8_t spray_mode; /*<  spray mode*/
}) mavlink_spray_value_t;

#define MAVLINK_MSG_ID_SPRAY_VALUE_LEN 10
#define MAVLINK_MSG_ID_SPRAY_VALUE_MIN_LEN 10
#define MAVLINK_MSG_ID_220_LEN 10
#define MAVLINK_MSG_ID_220_MIN_LEN 10

#define MAVLINK_MSG_ID_SPRAY_VALUE_CRC 229
#define MAVLINK_MSG_ID_220_CRC 229



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_SPRAY_VALUE { \
    220, \
    "SPRAY_VALUE", \
    4, \
    {  { "spray_speed", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_spray_value_t, spray_speed) }, \
         { "volume_sprayed", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_spray_value_t, volume_sprayed) }, \
         { "spray_state", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_spray_value_t, spray_state) }, \
         { "spray_mode", NULL, MAVLINK_TYPE_UINT8_T, 0, 9, offsetof(mavlink_spray_value_t, spray_mode) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_SPRAY_VALUE { \
    "SPRAY_VALUE", \
    4, \
    {  { "spray_speed", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_spray_value_t, spray_speed) }, \
         { "volume_sprayed", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_spray_value_t, volume_sprayed) }, \
         { "spray_state", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_spray_value_t, spray_state) }, \
         { "spray_mode", NULL, MAVLINK_TYPE_UINT8_T, 0, 9, offsetof(mavlink_spray_value_t, spray_mode) }, \
         } \
}
#endif

/**
 * @brief Pack a spray_value message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param spray_speed [ml/s] spray speed
 * @param volume_sprayed [ml] volume sprayed
 * @param spray_state  nowater state
 * @param spray_mode  spray mode
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_spray_value_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               float spray_speed, float volume_sprayed, uint8_t spray_state, uint8_t spray_mode)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_SPRAY_VALUE_LEN];
    _mav_put_float(buf, 0, spray_speed);
    _mav_put_float(buf, 4, volume_sprayed);
    _mav_put_uint8_t(buf, 8, spray_state);
    _mav_put_uint8_t(buf, 9, spray_mode);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_SPRAY_VALUE_LEN);
#else
    mavlink_spray_value_t packet;
    packet.spray_speed = spray_speed;
    packet.volume_sprayed = volume_sprayed;
    packet.spray_state = spray_state;
    packet.spray_mode = spray_mode;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_SPRAY_VALUE_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_SPRAY_VALUE;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_SPRAY_VALUE_MIN_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_CRC);
}

/**
 * @brief Pack a spray_value message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param spray_speed [ml/s] spray speed
 * @param volume_sprayed [ml] volume sprayed
 * @param spray_state  nowater state
 * @param spray_mode  spray mode
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_spray_value_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   float spray_speed,float volume_sprayed,uint8_t spray_state,uint8_t spray_mode)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_SPRAY_VALUE_LEN];
    _mav_put_float(buf, 0, spray_speed);
    _mav_put_float(buf, 4, volume_sprayed);
    _mav_put_uint8_t(buf, 8, spray_state);
    _mav_put_uint8_t(buf, 9, spray_mode);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_SPRAY_VALUE_LEN);
#else
    mavlink_spray_value_t packet;
    packet.spray_speed = spray_speed;
    packet.volume_sprayed = volume_sprayed;
    packet.spray_state = spray_state;
    packet.spray_mode = spray_mode;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_SPRAY_VALUE_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_SPRAY_VALUE;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_SPRAY_VALUE_MIN_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_CRC);
}

/**
 * @brief Encode a spray_value struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param spray_value C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_spray_value_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_spray_value_t* spray_value)
{
    return mavlink_msg_spray_value_pack(system_id, component_id, msg, spray_value->spray_speed, spray_value->volume_sprayed, spray_value->spray_state, spray_value->spray_mode);
}

/**
 * @brief Encode a spray_value struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param spray_value C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_spray_value_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_spray_value_t* spray_value)
{
    return mavlink_msg_spray_value_pack_chan(system_id, component_id, chan, msg, spray_value->spray_speed, spray_value->volume_sprayed, spray_value->spray_state, spray_value->spray_mode);
}

/**
 * @brief Send a spray_value message
 * @param chan MAVLink channel to send the message
 *
 * @param spray_speed [ml/s] spray speed
 * @param volume_sprayed [ml] volume sprayed
 * @param spray_state  nowater state
 * @param spray_mode  spray mode
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_spray_value_send(mavlink_channel_t chan, float spray_speed, float volume_sprayed, uint8_t spray_state, uint8_t spray_mode)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_SPRAY_VALUE_LEN];
    _mav_put_float(buf, 0, spray_speed);
    _mav_put_float(buf, 4, volume_sprayed);
    _mav_put_uint8_t(buf, 8, spray_state);
    _mav_put_uint8_t(buf, 9, spray_mode);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_SPRAY_VALUE, buf, MAVLINK_MSG_ID_SPRAY_VALUE_MIN_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_CRC);
#else
    mavlink_spray_value_t packet;
    packet.spray_speed = spray_speed;
    packet.volume_sprayed = volume_sprayed;
    packet.spray_state = spray_state;
    packet.spray_mode = spray_mode;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_SPRAY_VALUE, (const char *)&packet, MAVLINK_MSG_ID_SPRAY_VALUE_MIN_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_CRC);
#endif
}

/**
 * @brief Send a spray_value message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_spray_value_send_struct(mavlink_channel_t chan, const mavlink_spray_value_t* spray_value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_spray_value_send(chan, spray_value->spray_speed, spray_value->volume_sprayed, spray_value->spray_state, spray_value->spray_mode);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_SPRAY_VALUE, (const char *)spray_value, MAVLINK_MSG_ID_SPRAY_VALUE_MIN_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_CRC);
#endif
}

#if MAVLINK_MSG_ID_SPRAY_VALUE_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_spray_value_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  float spray_speed, float volume_sprayed, uint8_t spray_state, uint8_t spray_mode)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_float(buf, 0, spray_speed);
    _mav_put_float(buf, 4, volume_sprayed);
    _mav_put_uint8_t(buf, 8, spray_state);
    _mav_put_uint8_t(buf, 9, spray_mode);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_SPRAY_VALUE, buf, MAVLINK_MSG_ID_SPRAY_VALUE_MIN_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_CRC);
#else
    mavlink_spray_value_t *packet = (mavlink_spray_value_t *)msgbuf;
    packet->spray_speed = spray_speed;
    packet->volume_sprayed = volume_sprayed;
    packet->spray_state = spray_state;
    packet->spray_mode = spray_mode;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_SPRAY_VALUE, (const char *)packet, MAVLINK_MSG_ID_SPRAY_VALUE_MIN_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_LEN, MAVLINK_MSG_ID_SPRAY_VALUE_CRC);
#endif
}
#endif

#endif

// MESSAGE SPRAY_VALUE UNPACKING


/**
 * @brief Get field spray_speed from spray_value message
 *
 * @return [ml/s] spray speed
 */
static inline float mavlink_msg_spray_value_get_spray_speed(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field volume_sprayed from spray_value message
 *
 * @return [ml] volume sprayed
 */
static inline float mavlink_msg_spray_value_get_volume_sprayed(const mavlink_message_t* msg)
{
    return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field spray_state from spray_value message
 *
 * @return  nowater state
 */
static inline uint8_t mavlink_msg_spray_value_get_spray_state(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  8);
}

/**
 * @brief Get field spray_mode from spray_value message
 *
 * @return  spray mode
 */
static inline uint8_t mavlink_msg_spray_value_get_spray_mode(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  9);
}

/**
 * @brief Decode a spray_value message into a struct
 *
 * @param msg The message to decode
 * @param spray_value C-struct to decode the message contents into
 */
static inline void mavlink_msg_spray_value_decode(const mavlink_message_t* msg, mavlink_spray_value_t* spray_value)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    spray_value->spray_speed = mavlink_msg_spray_value_get_spray_speed(msg);
    spray_value->volume_sprayed = mavlink_msg_spray_value_get_volume_sprayed(msg);
    spray_value->spray_state = mavlink_msg_spray_value_get_spray_state(msg);
    spray_value->spray_mode = mavlink_msg_spray_value_get_spray_mode(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_SPRAY_VALUE_LEN? msg->len : MAVLINK_MSG_ID_SPRAY_VALUE_LEN;
        memset(spray_value, 0, MAVLINK_MSG_ID_SPRAY_VALUE_LEN);
    memcpy(spray_value, _MAV_PAYLOAD(msg), len);
#endif
}
