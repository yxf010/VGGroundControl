/*=====================================================================
 
 QGroundControl Open Source Ground Control Station
 
 (c) 2009 - 2015 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 
 This file is part of the QGROUNDCONTROL project
 
 QGROUNDCONTROL is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 QGROUNDCONTROL is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.
 
 ======================================================================*/

#ifndef SerialPortInfo_H
#define SerialPortInfo_H

#include "vgcomm_global.h"
#include <QLoggingCategory>

#ifdef __android__
    #include "qserialportinfo.h"
#else
    #include <QSerialPortInfo>
#endif


Q_DECLARE_LOGGING_CATEGORY(SerialPortInfoLog)

/// QGC's version of Qt QSerialPortInfo. It provides additional information about board types
/// that QGC cares about.
class VGCOMMSHARED_EXPORT SerialPortInfo : public QSerialPortInfo
{
public:
    typedef enum {
        BoardTypePX4FMUV1,
        BoardTypePX4FMUV2,
        BoardTypePX4FMUV4,
        BoardTypePX4Flow,
        BoardType3drRadio,
        BoardTypeAeroCore,
#ifndef __android__
        BoardTypeUBloxGPS, // Added by qianyong of VIGA
#endif
        BoardTypeUnknown
    } BoardType_t;

    // Vendor and products ids for the boards we care about

    static const int px4VendorId =                          9900;   ///< Vendor ID for Pixhawk board (V2 and V1) and PX4 Flow

    static const int pixhawkFMUV4ProductId =                18;     ///< Product ID for Pixhawk V2 board
    static const int pixhawkFMUV2ProductId =                17;     ///< Product ID for Pixhawk V2 board
    static const int pixhawkFMUV2OldBootloaderProductId =   22;     ///< Product ID for Bootloader on older Pixhawk V2 boards
    static const int pixhawkFMUV1ProductId =                16;     ///< Product ID for PX4 FMU V1 board

    static const int AeroCoreProductId =                    4097;   ///< Product ID for the AeroCore board
    
    static const int px4FlowProductId =                     21;     ///< Product ID for PX4 Flow board

    static const int threeDRRadioVendorId =                 1027;   ///< Vendor ID for 3DR Radio
    static const int threeDRRadioProductId =                24597;  ///< Product ID for 3DR Radio

#ifndef __android__
	// Added by qianyong of VIGA
	static const int ubloxGPSVendorId = 					5446;  	///< Vendor ID for UBlox GPS
	static const int ubloxGPSProductId = 					424;  	///< Product ID for UBlox GPS
#endif

    static const int uartBridgeVendorId =                   4292;   ///10C4，433MHZ无线电台串口vendor id;
    static const int uartBridgeProductId =                  60000;  ///EA60，433MHZ无线电台串口Product id;

    SerialPortInfo(void);
    SerialPortInfo(const QSerialPort & port);

    /// Override of QSerialPortInfo::availablePorts
    static QList<SerialPortInfo> availablePorts(void);

    BoardType_t boardType(void) const;

    /// @return true: board is a Pixhawk board
    bool boardTypePixhawk(void) const;

    /// @return true: Board is currently in bootloader
    bool isBootloader(void) const;

#ifndef __android__
	// Added by qianyong of VIGA
	bool boardTypeGPS(void) const;
	int gpsType(void) const;
#endif
};

#endif
