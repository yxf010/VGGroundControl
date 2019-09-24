#pragma once
// MESSAGE QX_ACCOUNT PACKING

#define MAVLINK_MSG_ID_QX_ACCOUNT 409

MAVPACKED(
typedef struct __mavlink_qx_account_t {
 uint8_t acc[16]; /*<  qx ntrip acc*/
 uint8_t pswd[16]; /*<  qx ntrip pswd*/
 uint8_t server; /*<  qx ntrip pswd*/
}) mavlink_qx_account_t;

#define MAVLINK_MSG_ID_QX_ACCOUNT_LEN 33
#define MAVLINK_MSG_ID_QX_ACCOUNT_MIN_LEN 33
#define MAVLINK_MSG_ID_409_LEN 33
#define MAVLINK_MSG_ID_409_MIN_LEN 33

#define MAVLINK_MSG_ID_QX_ACCOUNT_CRC 108
#define MAVLINK_MSG_ID_409_CRC 108

#define MAVLINK_MSG_QX_ACCOUNT_FIELD_ACC_LEN 16
#define MAVLINK_MSG_QX_ACCOUNT_FIELD_PSWD_LEN 16

#if MAVLINK_COMMAND_24BIT
#define MAVLINK_MESSAGE_INFO_QX_ACCOUNT { \
    409, \
    "QX_ACCOUNT", \
    3, \
    {  { "acc", NULL, MAVLINK_TYPE_UINT8_T, 16, 0, offsetof(mavlink_qx_account_t, acc) }, \
         { "pswd", NULL, MAVLINK_TYPE_UINT8_T, 16, 16, offsetof(mavlink_qx_account_t, pswd) }, \
         { "server", NULL, MAVLINK_TYPE_UINT8_T, 0, 32, offsetof(mavlink_qx_account_t, server) }, \
         } \
}
#else
#define MAVLINK_MESSAGE_INFO_QX_ACCOUNT { \
    "QX_ACCOUNT", \
    3, \
    {  { "acc", NULL, MAVLINK_TYPE_UINT8_T, 16, 0, offsetof(mavlink_qx_account_t, acc) }, \
         { "pswd", NULL, MAVLINK_TYPE_UINT8_T, 16, 16, offsetof(mavlink_qx_account_t, pswd) }, \
         { "server", NULL, MAVLINK_TYPE_UINT8_T, 0, 32, offsetof(mavlink_qx_account_t, server) }, \
         } \
}
#endif

/**
 * @brief Pack a qx_account message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param acc  qx ntrip acc
 * @param pswd  qx ntrip pswd
 * @param server  qx ntrip pswd
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_qx_account_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
                               const uint8_t *acc, const uint8_t *pswd, uint8_t server)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_QX_ACCOUNT_LEN];
    _mav_put_uint8_t(buf, 32, server);
    _mav_put_uint8_t_array(buf, 0, acc, 16);
    _mav_put_uint8_t_array(buf, 16, pswd, 16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_QX_ACCOUNT_LEN);
#else
    mavlink_qx_account_t packet;
    packet.server = server;
    mav_array_memcpy(packet.acc, acc, sizeof(uint8_t)*16);
    mav_array_memcpy(packet.pswd, pswd, sizeof(uint8_t)*16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_QX_ACCOUNT_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_QX_ACCOUNT;
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_QX_ACCOUNT_MIN_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_CRC);
}

/**
 * @brief Pack a qx_account message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param acc  qx ntrip acc
 * @param pswd  qx ntrip pswd
 * @param server  qx ntrip pswd
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_qx_account_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
                               mavlink_message_t* msg,
                                   const uint8_t *acc,const uint8_t *pswd,uint8_t server)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_QX_ACCOUNT_LEN];
    _mav_put_uint8_t(buf, 32, server);
    _mav_put_uint8_t_array(buf, 0, acc, 16);
    _mav_put_uint8_t_array(buf, 16, pswd, 16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_QX_ACCOUNT_LEN);
#else
    mavlink_qx_account_t packet;
    packet.server = server;
    mav_array_memcpy(packet.acc, acc, sizeof(uint8_t)*16);
    mav_array_memcpy(packet.pswd, pswd, sizeof(uint8_t)*16);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_QX_ACCOUNT_LEN);
#endif

    msg->msgid = MAVLINK_MSG_ID_QX_ACCOUNT;
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_QX_ACCOUNT_MIN_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_CRC);
}

/**
 * @brief Encode a qx_account struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param qx_account C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_qx_account_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_qx_account_t* qx_account)
{
    return mavlink_msg_qx_account_pack(system_id, component_id, msg, qx_account->acc, qx_account->pswd, qx_account->server);
}

/**
 * @brief Encode a qx_account struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param qx_account C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_qx_account_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_qx_account_t* qx_account)
{
    return mavlink_msg_qx_account_pack_chan(system_id, component_id, chan, msg, qx_account->acc, qx_account->pswd, qx_account->server);
}

/**
 * @brief Send a qx_account message
 * @param chan MAVLink channel to send the message
 *
 * @param acc  qx ntrip acc
 * @param pswd  qx ntrip pswd
 * @param server  qx ntrip pswd
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_qx_account_send(mavlink_channel_t chan, const uint8_t *acc, const uint8_t *pswd, uint8_t server)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char buf[MAVLINK_MSG_ID_QX_ACCOUNT_LEN];
    _mav_put_uint8_t(buf, 32, server);
    _mav_put_uint8_t_array(buf, 0, acc, 16);
    _mav_put_uint8_t_array(buf, 16, pswd, 16);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_QX_ACCOUNT, buf, MAVLINK_MSG_ID_QX_ACCOUNT_MIN_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_CRC);
#else
    mavlink_qx_account_t packet;
    packet.server = server;
    mav_array_memcpy(packet.acc, acc, sizeof(uint8_t)*16);
    mav_array_memcpy(packet.pswd, pswd, sizeof(uint8_t)*16);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_QX_ACCOUNT, (const char *)&packet, MAVLINK_MSG_ID_QX_ACCOUNT_MIN_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_CRC);
#endif
}

/**
 * @brief Send a qx_account message
 * @param chan MAVLink channel to send the message
 * @param struct The MAVLink struct to serialize
 */
static inline void mavlink_msg_qx_account_send_struct(mavlink_channel_t chan, const mavlink_qx_account_t* qx_account)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_qx_account_send(chan, qx_account->acc, qx_account->pswd, qx_account->server);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_QX_ACCOUNT, (const char *)qx_account, MAVLINK_MSG_ID_QX_ACCOUNT_MIN_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_CRC);
#endif
}

#if MAVLINK_MSG_ID_QX_ACCOUNT_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_qx_account_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  const uint8_t *acc, const uint8_t *pswd, uint8_t server)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    char *buf = (char *)msgbuf;
    _mav_put_uint8_t(buf, 32, server);
    _mav_put_uint8_t_array(buf, 0, acc, 16);
    _mav_put_uint8_t_array(buf, 16, pswd, 16);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_QX_ACCOUNT, buf, MAVLINK_MSG_ID_QX_ACCOUNT_MIN_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_CRC);
#else
    mavlink_qx_account_t *packet = (mavlink_qx_account_t *)msgbuf;
    packet->server = server;
    mav_array_memcpy(packet->acc, acc, sizeof(uint8_t)*16);
    mav_array_memcpy(packet->pswd, pswd, sizeof(uint8_t)*16);
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_QX_ACCOUNT, (const char *)packet, MAVLINK_MSG_ID_QX_ACCOUNT_MIN_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_LEN, MAVLINK_MSG_ID_QX_ACCOUNT_CRC);
#endif
}
#endif

#endif

// MESSAGE QX_ACCOUNT UNPACKING


/**
 * @brief Get field acc from qx_account message
 *
 * @return  qx ntrip acc
 */
static inline uint16_t mavlink_msg_qx_account_get_acc(const mavlink_message_t* msg, uint8_t *acc)
{
    return _MAV_RETURN_uint8_t_array(msg, acc, 16,  0);
}

/**
 * @brief Get field pswd from qx_account message
 *
 * @return  qx ntrip pswd
 */
static inline uint16_t mavlink_msg_qx_account_get_pswd(const mavlink_message_t* msg, uint8_t *pswd)
{
    return _MAV_RETURN_uint8_t_array(msg, pswd, 16,  16);
}

/**
 * @brief Get field server from qx_account message
 *
 * @return  qx ntrip pswd
 */
static inline uint8_t mavlink_msg_qx_account_get_server(const mavlink_message_t* msg)
{
    return _MAV_RETURN_uint8_t(msg,  32);
}

/**
 * @brief Decode a qx_account message into a struct
 *
 * @param msg The message to decode
 * @param qx_account C-struct to decode the message contents into
 */
static inline void mavlink_msg_qx_account_decode(const mavlink_message_t* msg, mavlink_qx_account_t* qx_account)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
    mavlink_msg_qx_account_get_acc(msg, qx_account->acc);
    mavlink_msg_qx_account_get_pswd(msg, qx_account->pswd);
    qx_account->server = mavlink_msg_qx_account_get_server(msg);
#else
        uint8_t len = msg->len < MAVLINK_MSG_ID_QX_ACCOUNT_LEN? msg->len : MAVLINK_MSG_ID_QX_ACCOUNT_LEN;
        memset(qx_account, 0, MAVLINK_MSG_ID_QX_ACCOUNT_LEN);
    memcpy(qx_account, _MAV_PAYLOAD(msg), len);
#endif
}
