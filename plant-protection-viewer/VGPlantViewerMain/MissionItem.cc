/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#include <QStringList>
#include <QDebug>

#include "MissionItem.h"
#include "VGApplication.h"

MissionItem::MissionItem(QObject* parent)
    : QObject(parent), m_sequenceNumber(0)
    , m_doJumpId(-1), m_isCurrentItem(false)
	, m_autoContinue(true), m_command(MAV_CMD(0))
	, m_frame(MAV_FRAME_GLOBAL)
    , m_missionType(MAV_MISSION_TYPE_MISSION)
{
}

MissionItem::MissionItem( int                sequenceNumber,
                          MAV_CMD            command,
                          MAV_FRAME          frame,
                          const QVariant     &param1,
                          const QVariant     &param2,
                          const QVariant     &param3,
                          const QVariant     &param4,
                          const QVariant     &param5,
                          const QVariant     &param6,
                          const QVariant     &param7,
                          bool               autoContinue,
                          bool               isCurrentItem,
                          MAV_MISSION_TYPE   tp,
                          QObject*           parent )
    : QObject(parent), m_sequenceNumber(sequenceNumber)
    , m_doJumpId(-1), m_isCurrentItem(isCurrentItem)
	, m_command(command), m_frame(frame)
	, m_autoContinue(autoContinue)
    , m_missionType(tp)
{
	m_params[0] = param1;
	m_params[1] = param2;
	m_params[2] = param3;
	m_params[3] = param4;
	m_params[4] = param5;
	m_params[5] = param6;
	m_params[6] = param7;
}

MissionItem::MissionItem(const MissionItem& other, QObject* parent)
    : QObject(parent)
{
    *this = other;
}

MissionItem &MissionItem::operator=(const MissionItem& other)
{
    m_doJumpId = other.m_doJumpId;

    setCommand(other.command());
    setFrame(other.frame());
    setSequenceNumber(other.m_sequenceNumber);
    setAutoContinue(other.autoContinue());
    setIsCurrentItem(other.m_isCurrentItem);
    m_missionType = other.m_missionType;
    for (size_t i = 0; i < 7; i++)
    {
        m_params[i] = other.m_params[i];
    }

    return *this;
}

MissionItem::~MissionItem()
{
}

MAV_CMD MissionItem::command(void) const
{
	return m_command;
}

bool MissionItem::isCurrentItem(void) const
{
	return m_isCurrentItem;
}

int MissionItem::sequenceNumber(void) const
{
	return m_sequenceNumber;
}

MAV_FRAME MissionItem::frame(void) const
{
	return m_frame;
}

bool MissionItem::autoContinue(void) const
{
	return m_autoContinue;
}

QVariant MissionItem::param1(void) const
{
	return m_params[0];
}

QVariant MissionItem::param2(void) const
{
	return m_params[1];
}

QVariant MissionItem::param3(void) const
{
	return m_params[2];
}

QVariant MissionItem::param4(void) const
{
	return m_params[3];
}

QVariant MissionItem::param5(void) const
{
	return m_params[4];
}

QVariant MissionItem::param6(void) const
{
	return m_params[5];
}

QVariant MissionItem::param7(void) const
{
	return m_params[6];
}

void MissionItem::setSequenceNumber(int sequenceNumber)
{
    if (m_sequenceNumber != sequenceNumber) {
        m_sequenceNumber = sequenceNumber;
        emit sequenceNumberChanged(m_sequenceNumber);
    }
}

void MissionItem::setCommand(MAV_CMD command)
{
    if ((MAV_CMD)this->command() != command) {
        m_command = command;
    }
}

void MissionItem::setFrame(MAV_FRAME frame)
{
    if (this->frame() != frame) {
        m_frame = frame;
    }
}

void MissionItem::setAutoContinue(bool autoContinue)
{
    if (this->autoContinue() != autoContinue) {
        m_autoContinue = autoContinue;
    }
}

void MissionItem::setIsCurrentItem(bool isCurrentItem)
{
    if (m_isCurrentItem != isCurrentItem) {
        m_isCurrentItem = isCurrentItem;
        emit isCurrentItemChanged(isCurrentItem);
    }
}

void MissionItem::setParam1(const QVariant  &param)
{
	m_params[0] = param;
}

void MissionItem::setParam2(const QVariant  &param)
{
	m_params[1] = param;
}

void MissionItem::setParam3(const QVariant  &param)
{
	m_params[2] = param;
}

void MissionItem::setParam4(const QVariant  &param)
{
	m_params[3] = param;
}

void MissionItem::setParam5(const QVariant  &param)
{
	m_params[4] = param;
}

void MissionItem::setParam6(const QVariant  &param)
{
	m_params[5] = param;
}

void MissionItem::setParam7(const QVariant  &param)
{
	m_params[6] = param;
}

void MissionItem::setCoordinate(const QGeoCoordinate& coordinate)
{
    setParam5(coordinate.latitude());
    setParam6(coordinate.longitude());
    setParam7(coordinate.altitude());
}

bool MissionItem::relativeAltitude(void) const
{
    return frame() == MAV_FRAME_GLOBAL_RELATIVE_ALT;
}

QGeoCoordinate MissionItem::coordinate(void) const
{
    return QGeoCoordinate(param5().toDouble(), param6().toDouble(), param7().toDouble());
}

int MissionItem::doJumpId() const
{
    return m_doJumpId;
}

MAV_MISSION_TYPE MissionItem::missionType() const
{
    return m_missionType;
}

double MissionItem::specifiedFlightSpeed(void) const
{
    double flightSpeed = std::numeric_limits<double>::quiet_NaN();

    if (m_command == MAV_CMD_DO_CHANGE_SPEED && param2() > 0)
		return param2().toDouble();

    return flightSpeed;
}

double MissionItem::specifiedGimbalYaw(void) const
{
    double gimbalYaw = std::numeric_limits<double>::quiet_NaN();

    if (m_command == MAV_CMD_DO_MOUNT_CONTROL && (int)param7().toInt() == MAV_MOUNT_MODE_MAVLINK_TARGETING) {
        gimbalYaw = param3().toDouble();
    }

    return gimbalYaw;
}

void MissionItem::_param2Changed(QVariant value)
{
    Q_UNUSED(value);

    double flightSpeed = specifiedFlightSpeed();
    if (!qIsNaN(flightSpeed)) {
        emit specifiedFlightSpeedChanged(flightSpeed);
    }
}

void MissionItem::_param3Changed(QVariant value)
{
    Q_UNUSED(value);

    double gimbalYaw = specifiedGimbalYaw();
    if (!qIsNaN(gimbalYaw)) {
        emit specifiedGimbalYawChanged(gimbalYaw);
    }
}
