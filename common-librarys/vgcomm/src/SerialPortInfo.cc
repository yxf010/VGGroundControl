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

#include "SerialPortInfo.h"

SerialPortInfo::SerialPortInfo(void) :
    QSerialPortInfo()
{

}

SerialPortInfo::SerialPortInfo(const QSerialPort & port) :
    QSerialPortInfo(port)
{

}

SerialPortInfo::BoardType_t SerialPortInfo::boardType(void) const
{
    if (isNull()) {
        return BoardTypeUnknown;
    }

    BoardType_t boardType = BoardTypeUnknown;

    switch (vendorIdentifier()) {
        case px4VendorId:
            if (productIdentifier() == pixhawkFMUV4ProductId) {
                qDebug() << "Found PX4 FMU V4";
                boardType = BoardTypePX4FMUV4;
            } else if (productIdentifier() == pixhawkFMUV2ProductId || productIdentifier() == pixhawkFMUV2OldBootloaderProductId) {
                qDebug() << "Found PX4 FMU V2";
                boardType = BoardTypePX4FMUV2;
            } else if (productIdentifier() == pixhawkFMUV1ProductId) {
                qDebug() << "Found PX4 FMU V1";
                boardType = BoardTypePX4FMUV1;
            } else if (productIdentifier() == px4FlowProductId) {
                qDebug() << "Found PX4 Flow";
                boardType = BoardTypePX4Flow;
            } else if (productIdentifier() == AeroCoreProductId) {
                qDebug() << "Found AeroCore";
                boardType = BoardTypeAeroCore;
            }
            break;
        case threeDRRadioVendorId:
            if (productIdentifier() == threeDRRadioProductId) {
                qDebug() << "Found SiK Radio";
                boardType = BoardType3drRadio;

            }
            break;
#ifndef __android__
        case ubloxGPSVendorId: // Added by qianyong of VIGA
            if (productIdentifier() == ubloxGPSProductId) {
                //qDebug() << "Found UBlox GPS";
                boardType = BoardTypeUBloxGPS;
            }
            break;
#endif
    case uartBridgeVendorId:
        if(productIdentifier() == uartBridgeProductId)
        {
            boardType = BoardType3drRadio;
        }
        break;
    }

    if (boardType == BoardTypeUnknown) {
        // Fall back to port name matching which could lead to incorrect board mapping. But in some cases the
        // vendor and product id do not come through correctly so this is used as a last chance detection method.
        if (description() == "PX4 FMU v4.x" || description() == "PX4 BL FMU v4.x") {
            qDebug() << "Found PX4 FMU V4 (by name matching fallback)";
            boardType = BoardTypePX4FMUV4;
        } else if (description() == "PX4 FMU v2.x" || description() == "PX4 BL FMU v2.x") {
            qDebug() << "Found PX4 FMU V2 (by name matching fallback)";
            boardType = BoardTypePX4FMUV2;
        } else if (description() == "PX4 FMU v1.x" || description() == "PX4 BL FMU v1.x") {
            qDebug() << "Found PX4 FMU V1 (by name matching fallback)";
            boardType = BoardTypePX4FMUV1;
        } else if (description().startsWith("PX4 FMU")) {
            qDebug() << "Found PX4 FMU, assuming V2 (by name matching fallback)";
            boardType = BoardTypePX4FMUV2;
        } else if (description().contains(QRegExp("PX4.*Flow", Qt::CaseInsensitive))) {
            qDebug() << "Found possible px4 flow camera (by name matching fallback)";
            boardType = BoardTypePX4Flow;
        } else if (description() == "FT231X USB UART") {
            qDebug() << "Found possible Radio (by name matching fallback)";
            boardType = BoardType3drRadio;
#ifdef __android__
        } else if (description().endsWith("USB UART")) {
            // This is a fairly broad fallbacks for radios which will also catch most FTDI devices. That would
            // cause problems on desktop due to incorrect connections. Since mobile is more anal about connecting
            // it will work fine here.
            boardType = BoardType3drRadio;
#endif
		//add by wkr 2016-04-21 + {{
#ifndef __android__
		}
		else if (description().contains("u-blox"))
		{
            //qDebug() << "Found UBlox GPS";
			boardType = BoardTypeUBloxGPS;
#endif // !__android__
		}
        else if(description().contains("CP201x USB"))
        {
            boardType = BoardType3drRadio;
        }
		//}}
    }

    return boardType;
}

QList<SerialPortInfo> SerialPortInfo::availablePorts(void)
{
    QList<SerialPortInfo>    list;

    foreach(QSerialPortInfo portInfo, QSerialPortInfo::availablePorts()) {
        list << *((SerialPortInfo*)&portInfo);
    }

    return list;
}

bool SerialPortInfo::boardTypePixhawk(void) const
{
    BoardType_t boardType = this->boardType();

    return boardType == BoardTypePX4FMUV1 || boardType == BoardTypePX4FMUV2 || boardType == BoardTypePX4FMUV4 || boardType == BoardTypeAeroCore;
}

bool SerialPortInfo::isBootloader(void) const
{
    // FIXME: Check SerialLink bootloade detect code which is different
    return boardTypePixhawk() && description().contains("BL");
}

#ifndef __android__
// Added by qianyong of VIGA
bool SerialPortInfo::boardTypeGPS(void) const
{
    BoardType_t boardType = this->boardType();

	return boardType == BoardTypeUBloxGPS;
}

// Added by qianyong of VIGA
// 0-others,1-ublox,...
int SerialPortInfo::gpsType(void) const
{
    BoardType_t boardType = this->boardType();

	if (boardType == BoardTypeUBloxGPS) {
		return 1;
	} else {
		return 0;
	}
}
#endif

