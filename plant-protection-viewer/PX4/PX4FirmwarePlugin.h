/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


/// @file
///     @author Don Gagne <don@thegagnes.com>

#ifndef PX4FirmwarePlugin_H
#define PX4FirmwarePlugin_H

#include "FirmwarePlugin.h"
#include "ParameterManager.h"
#include "PX4ParameterMetaData.h"
#include "PX4GeoFenceManager.h"

class PX4FirmwarePlugin : public FirmwarePlugin
{
    Q_OBJECT

public:
    PX4FirmwarePlugin(void);

    // Overrides from FirmwarePlugin

    QList<VehicleComponent*> componentsForVehicle(AutoPilotPlugin* vehicle) override;
    QList<MAV_CMD> supportedMissionCommands(void) override;

    AutoPilotPlugin*    autopilotPlugin                 (VGVehicle* vehicle) override;
    bool                isCapable                       (const VGVehicle *vehicle, FirmwareCapabilities capabilities) override;
    QStringList         flightModes                     (VGVehicle* vehicle) override;
    QString             flightMode                      (uint8_t base_mode, uint32_t custom_mode) const override;
    bool                setFlightMode                   (const QString& flightMode, uint8_t* base_mode, uint32_t* custom_mode) override;
    void                setGuidedMode                   (VGVehicle* vehicle, bool guidedMode) override;
    QString             pauseFlightMode                 (void) const override { return _holdFlightMode; }
    QString             missionFlightMode               (void) const override { return _missionFlightMode; }
    QString             rtlFlightMode                   (void) const override { return _rtlFlightMode; }
    QString             landFlightMode                  (void) const override { return _landingFlightMode; }
    QString             takeControlFlightMode           (void) const override { return _manualFlightMode; }
    void                pauseVehicle                    (VGVehicle* vehicle) override;
    void                guidedModeRTL                   (VGVehicle* vehicle) override;
    void                guidedModeLand                  (VGVehicle* vehicle) override;
    void                guidedModeTakeoff               (VGVehicle* vehicle) override;
    void                guidedModeOrbit                 (VGVehicle* vehicle, const QGeoCoordinate& centerCoord = QGeoCoordinate(), double radius = NAN, double velocity = NAN, double altitude = NAN) override;
    void                guidedModeGotoLocation          (VGVehicle* vehicle, const QGeoCoordinate& gotoCoord) override;
    void                guidedModeChangeAltitude        (VGVehicle* vehicle, double altitudeRel) override;
    void                startMission                    (VGVehicle* vehicle) override;
    bool                isGuidedMode                    (const VGVehicle* vehicle) const override;
    int                 manualControlReservedButtonCount(void) override;
    bool                supportsManualControl           (void) override;
    void                initializeVehicle               (VGVehicle* vehicle) override;
    bool                sendHomePositionToVehicle       (void) override;
    void                addMetaDataToFact               (QObject* parameterMetaData, Fact* fact, MAV_TYPE vehicleType) override;
    QString             missionCommandOverrides         (MAV_TYPE vehicleType) const override;
    QString             getVersionParam                 (void) override { return QString("SYS_PARAM_VER"); }
    QString             internalParameterMetaDataFile   (VGVehicle* vehicle) override { Q_UNUSED(vehicle); return QString(":/FirmwarePlugin/PX4/PX4ParameterFactMetaData.xml"); }
    void                getParameterMetaDataVersionInfo (const QString& metaDataFile, int& majorVersion, int& minorVersion) override { PX4ParameterMetaData::getParameterMetaDataVersionInfo(metaDataFile, majorVersion, minorVersion); }
    QObject*            loadParameterMetaData           (const QString& metaDataFile) final;
    bool                adjustIncomingMavlinkMessage    (VGVehicle* vehicle, mavlink_message_t* message) override;
    GeoFenceManager*    newGeoFenceManager              (VGVehicle* vehicle) override { return new PX4GeoFenceManager(vehicle); }
    QString             offlineEditingParamFile(VGVehicle* vehicle) override { Q_UNUSED(vehicle); return QStringLiteral(":/FirmwarePlugin/PX4/PX4.OfflineEditing.params"); }
    QString             brandImageIndoor                (const VGVehicle* vehicle) const override { Q_UNUSED(vehicle); return QStringLiteral("/qmlimages/PX4/BrandImage"); }
    QString             brandImageOutdoor               (const VGVehicle* vehicle) const override { Q_UNUSED(vehicle); return QStringLiteral("/qmlimages/PX4/BrandImage"); }
    bool                vehicleYawsToNextWaypointInMission(const VGVehicle* vehicle) const override;
    QString             autoDisarmParameter             (VGVehicle* vehicle) override { Q_UNUSED(vehicle); return QStringLiteral("COM_DISARM_LAND"); }

protected:
    typedef struct {
        uint8_t         main_mode;
        uint8_t         sub_mode;
        const QString*  name;       ///< Name for flight mode
        bool            canBeSet;   ///< true: VGVehicle can be set to this flight mode
        bool            fixedWing;  /// fixed wing compatible
        bool            multiRotor; /// multi rotor compatible
    } FlightModeInfo_t;

    QList<FlightModeInfo_t> _flightModeInfoList;

    // Use these constants to set flight modes using setFlightMode method. Don't use hardcoded string names since the
    // names may change.

    // If plugin superclass wants to change a mode name, then set a new name for the flight mode in the superclass constructor
    QString _manualFlightMode;
    QString _acroFlightMode;
    QString _stabilizedFlightMode;
    QString _rattitudeFlightMode;
    QString _altCtlFlightMode;
    QString _posCtlFlightMode;
    QString _offboardFlightMode;
    QString _readyFlightMode;
    QString _takeoffFlightMode;
    QString _holdFlightMode;
    QString _missionFlightMode;
    QString _rtlFlightMode;
    QString _landingFlightMode;
    QString _rtgsFlightMode;
    QString _followMeFlightMode;
    QString _simpleFlightMode;

private slots:
    void _mavCommandResult(int vehicleId, int component, int command, int result, bool noReponseFromVehicle);

private:
    void _handleAutopilotVersion(VGVehicle* vehicle, mavlink_message_t* message);

    // Any instance data here must be global to all vehicles
    // VGVehicle specific data should go into PX4FirmwarePluginInstanceData
};

class PX4FirmwarePluginInstanceData : public QObject
{
    Q_OBJECT

public:
    PX4FirmwarePluginInstanceData(QObject* parent = NULL);

    bool versionNotified;  ///< true: user notified over version issue
};

#endif
