#include "LinkInterface.h"
#include "LinkCommand.h"
#include <QDateTime>
#include <QDebug>

/*////////////////////////////////////////////////////////////////////////////////
LinkInterface
*/////////////////////////////////////////////////////////////////////////////////
LinkInterface::LinkInterface(LinkCommand *cmd) :
QObject(cmd), _decodedFirstMavlinkPacket(false)
, m_mavlinkChannelSet(false)
, m_active(false)
, _enableRateCollection(false)
, m_mavlinkChannel(0)
{
	// Initialize everything for the data rate calculation buffers.
	_inDataIndex = 0;
	_outDataIndex = 0;

	// Initialize our data rate buffers.
	memset(_inDataWriteAmounts, 0, sizeof(_inDataWriteAmounts));
	memset(_inDataWriteTimes, 0, sizeof(_inDataWriteTimes));
	memset(_outDataWriteAmounts, 0, sizeof(_outDataWriteAmounts));
	memset(_outDataWriteTimes, 0, sizeof(_outDataWriteTimes));
	qRegisterMetaType<LinkInterface*>("LinkInterface*");
    if (cmd)
        connect(this, &LinkInterface::connected, cmd, &LinkCommand::connectedChanged);
}

bool LinkInterface::active(void) const
{
    return m_active;
}

void LinkInterface::setActive(bool active)
{
    m_active = active;
    emit activeChanged(active);
}

LinkCommand *LinkInterface::getLinkCommand() const
{
    return dynamic_cast<LinkCommand*>(parent());
}

QString LinkInterface::devName() const
{
    if (LinkCommand *cmd = dynamic_cast<LinkCommand*>(parent()))
        return cmd->getName();

    return QString();
}

bool LinkInterface::isLogReplay(void) const
{
    return false;
}

uint8_t LinkInterface::getMavlinkChannel(void) const
{
    if (!m_mavlinkChannelSet)
        qWarning() << "Call to LinkInterface::mavlinkChannel with _mavlinkChannelSet == false";

    return m_mavlinkChannel;
}

int LinkInterface::linkType() const
{
	if (LinkCommand *lc = getLinkCommand())
		return lc->type();

	return LinkCommand::TypeLast;
}

bool LinkInterface::decodedFirstMavlinkPacket() const
{
    return _decodedFirstMavlinkPacket;
}

void LinkInterface::setDecodedFirstMavlinkPacket(bool b)
{
    _decodedFirstMavlinkPacket = b;
}

void LinkInterface::_logInputDataRate(quint64 byteCount, qint64 time)
{
    if(_enableRateCollection)
        _logDataRateToBuffer(_inDataWriteAmounts, _inDataWriteTimes, &_inDataIndex, byteCount, time);
}

void LinkInterface::_logOutputDataRate(quint64 byteCount, qint64 time)
{
    if (_enableRateCollection)
        _logDataRateToBuffer(_outDataWriteAmounts, _outDataWriteTimes, &_outDataIndex, byteCount, time);
}

void LinkInterface::_logDataRateToBuffer(quint64 *bytesBuffer, qint64 *timeBuffer, int *writeIndex, quint64 bytes, qint64 time)
{
	QMutexLocker dataRateLocker(&_dataRateMutex);

	int i = *writeIndex;

	bytesBuffer[i] = bytes;
	timeBuffer[i] = time;

	++i;
	if (i == DataRateBufferSize)
		i = 0;

	*writeIndex = i;
}
/// Sets the mavlink channel to use for this link
void LinkInterface::_setMavlinkChannel(uint8_t channel)
{
    if (m_mavlinkChannelSet)
        qWarning() << "Mavlink channel set multiple times";

    m_mavlinkChannelSet = true;
    m_mavlinkChannel = channel;
}
