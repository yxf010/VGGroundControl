#pragma once
// MESSAGE VIGA_EVENT PACKING

#define MAVLINK_MSG_ID_VIGA_EVENT 401

MAVPACKED(
typedef struct __mavlink_viga_event_t {
 int32_t viga_event; /*<  viga defines event*/
}) mavlink_viga_event_t;

#define MAVLINK_MSG_ID_VIGA_EVENT_LEN 4
#define MAVLINK_MSG_ID_VIGA_EVENT_MIN_LEN 4
#define MAVLINK_MSG_ID_401_LEN 4
#define MAVLINK_MSG_ID_401_MIN_LEN 4

#define MAVLINK_MSG_ID_VIGA_EVENT_CRC 225
#define MAVLINK_MSG_ID_401_CRC 225



#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_VIGA_EVENT { \
    401, \
    "VIGA_EVENT", \
    1, \
    {  { "viga_event", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_viga_event_t, viga_event) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_VIGA_EVENT { \
    "VIGA_EVENT", \
    1, \
    {  { "viga_event", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_viga_event_t, viga_event) }, \
         } \
}
#endif

/**
 * @brief Pack a viga_event message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param viga_event  viga defines event
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_viga_event_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               int32_t viga_event)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_VIGA_EVENT_LEN];
    _mav_put_int32_t(buf, 0, viga_event);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_VIGA_EVENT_LEN);
#else
    mavlink_viga_event_t packet;
    packet.viga_event = viga_event;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_VIGA_EVENT_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_VIGA_EVENT;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_VIGA_EVENT_MIN_LEN, MAVLINK_MSG_ID_VIGA_EVENT_LEN, MAVLINK_MSG_ID_VIGA_EVENT_CRC);
}

/**
 * @brief Pack a viga_event message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param viga_event  viga defines event
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_viga_event_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   int32_t viga_event)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_VIGA_EVENT_LEN];
    _mav_put_int32_t(buf, 0, viga_event);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_VIGA_EVENT_LEN);
#else
    mavlink_viga_event_t packet;
    packet.viga_event = viga_event;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_VIGA_EVENT_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_VIGA_EVENT;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_VIGA_EVENT_MIN_LEN, MAVLINK_MSG_ID_VIGA_EVENT_LEN, MAVLINK_MSG_ID_VIGA_EVENT_CRC);
}

/**
 * @brief Encode a viga_event struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param viga_event C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_viga_event_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_viga_event_t* viga_event)
{
    return mavlink_msg_viga_event_pack(system_id, component_id, msg, viga_event->viga_event);
}

/**
 * @brief Encode a viga_event struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param viga_event C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_viga_event_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_viga_event_t* viga_event)
{
    return mavlink_msg_viga_event_pack_chan(system_id, component_id, chan, msg, viga_event->viga_event);
}

/**
 * @brief Send a viga_event message
 * @param chan MAVLink channel to send the message
 *
 * @param viga_event  viga defines event
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_viga_event_send(mavlink_channel_t chan, int32_t viga_event)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_VIGA_EVENT_LEN];
    _mav_put_int32_t(buf, 0, viga_event);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_VIGA_EVENT, buf, MAVLINK_MSG_ID_VIGA_EVENT_MIN_LEN, MAVLINK_MSG_ID_VIGA_EVENT_LEN, MAVLINK_MSG_ID_VIGA_EVENT_CRC);
#else
    mavlink_viga_event_t packet;
    packet.viga_event = viga_event;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_VIGA_EVENT, (const char *)&packet, MAVLINK_MSG_ID_VIGA_EVENT_MIN_LEN, MAVLINK_MSG_ID_VIGA_EVENT_LEN, MAVLINK_MSG_ID_VIGA_EVENT_CRC);
#endif
}

/**
 * @brief Send a viga_event message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_viga_event_send_struct(mavlink_channel_t chan, const mavlink_viga_event_t* viga_event)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_viga_event_send(chan, viga_event->viga_event);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_VIGA_EVENT, (const char *)viga_event, MAVLINK_MSG_ID_VIGA_EVENT_MIN_LEN, MAVLINK_MSG_ID_VIGA_EVENT_LEN, MAVLINK_MSG_ID_VIGA_EVENT_CRC);
#endif
}

#if MAVLINK_MSG_ID_VIGA_EVENT_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_viga_event_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  int32_t viga_event)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_int32_t(buf, 0, viga_event);

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_VIGA_EVENT, buf, MAVLINK_MSG_ID_VIGA_EVENT_MIN_LEN, MAVLINK_MSG_ID_VIGA_EVENT_LEN, MAVLINK_MSG_ID_VIGA_EVENT_CRC);
#else
    mavlink_viga_event_t *packet = (mavlink_viga_event_t *)msgbuf;
    packet->viga_event = viga_event;

    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_VIGA_EVENT, (const char *)packet, MAVLINK_MSG_ID_VIGA_EVENT_MIN_LEN, MAVLINK_MSG_ID_VIGA_EVENT_LEN, MAVLINK_MSG_ID_VIGA_EVENT_CRC);
#endif
}
#endif

#endif

// MESSAGE VIGA_EVENT UNPACKING


/**
 * @brief Get field viga_event from viga_event message
 *
 * @return  viga defines event
 */
static inline int32_t mavlink_msg_viga_event_get_viga_event(const mavlink_message_t* msg)
{
    return _MAV_RETURN_int32_t(msg,  0);
}

/**
 * @brief Decode a viga_event message into a struct
 *
 * @param msg The message to decode
 * @param viga_event C-struct to decode the message contents into
 */
static inline void mavlink_msg_viga_event_decode(const mavlink_message_t* msg, mavlink_viga_event_t* viga_event)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    viga_event->viga_event = mavlink_msg_viga_event_get_viga_event(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_VIGA_EVENT_LEN? msg->len : MAVLINK_MSG_ID_VIGA_EVENT_LEN;
        memset(viga_event, 0, MAVLINK_MSG_ID_VIGA_EVENT_LEN);
    memcpy(viga_event, _MAV_PAYLOAD(msg), len);
#endif
}
