/*=====================================================================

PIXHAWK Micro Air Vehicle Flying Robotics Toolkit

(c) 2009 PIXHAWK PROJECT  <http://pixhawk.ethz.ch>

This file is part of the PIXHAWK project

PIXHAWK is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

PIXHAWK is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PIXHAWK. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

/**
* @file
*   @brief Brief Description
*
*   @author Lorenz Meier <mavteam@student.ethz.ch>
*
*/

#ifndef _LINKINTERFACE_H_
#define _LINKINTERFACE_H_

#include <QObject>
#include <QMutex>
#include "vgcomm_global.h"

class LinkCommand;
class VGCOMMSHARED_EXPORT LinkInterface : public QObject
{
    Q_OBJECT
    enum 
    {
        DataRateBufferSize = 20,
        DataRateCurrentTimespan = 500,
    };
public:
    // Property accessors
    bool active(void)const;
    void setActive(bool active);
	LinkCommand *getLinkCommand()const;
    virtual void requestReset() = 0;
    virtual bool isConnected() const = 0;
    virtual qint64 getConnectionSpeed() const = 0;
    virtual bool isLogReplay(void)const;
    void enableDataRate(bool enable);
	uint8_t getMavlinkChannel(void) const;

	int linkType()const;
    bool decodedFirstMavlinkPacket()const;
    void setDecodedFirstMavlinkPacket(bool b);
    QString devName()const;
public slots:
    virtual void writeBytes(const QByteArray &array) = 0;
protected:
	LinkInterface(LinkCommand *cmd);
    void _logInputDataRate(quint64 byteCount, qint64 time);
    void _logOutputDataRate(quint64 byteCount, qint64 time);
    virtual bool _connect(void) = 0;
    virtual void _disconnect(void) = 0;
signals:
    void autoconnectChanged(bool autoconnect);
    void activeChanged(bool active);
    void bytesReceived(LinkInterface* link, QByteArray data);
    void connected(bool b);
    void communicationError(const QString& title, const QString& error);
    void communicationUpdate(const QString& linkname, const QString& text);
private:
    void _logDataRateToBuffer(quint64 *bytesBuffer, qint64 *timeBuffer, int *writeIndex, quint64 bytes, qint64 time);
    /// Sets the mavlink channel to use for this link
    void _setMavlinkChannel(uint8_t channel);
private:
    friend class LinkCommand;
    friend class LinkManager;

    bool        m_mavlinkChannelSet;    ///< true: _mavlinkChannel has been set
    uint8_t     m_mavlinkChannel;    ///< mavlink channel to use for this link, as used by mavlink_parse_char
    int         _inDataIndex;
    quint64     _inDataWriteAmounts[DataRateBufferSize];   // In bytes
    qint64      _inDataWriteTimes[DataRateBufferSize];     // in ms
    int         _outDataIndex;
    quint64     _outDataWriteAmounts[DataRateBufferSize]; // In bytes
    qint64      _outDataWriteTimes[DataRateBufferSize]; // in ms
    bool        m_active;       ///< true: link is actively receiving mavlink messages
    bool        _enableRateCollection;
    bool        _decodedFirstMavlinkPacket;

    mutable QMutex _dataRateMutex; // Mutex for accessing the data rate member variables
};

#endif // _LINKINTERFACE_H_
