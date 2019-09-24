/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


#ifndef MissionItem_H
#define MissionItem_H

#include <QObject>
#include <QString>
#include <QtQml>
#include <QTextStream>
#include <QJsonObject>
#include <QGeoCoordinate>

#include "mavlink.h"
//#include "QGC.h"
//#include "MavlinkQmlSingleton.h"
#include "QmlObjectListModel.h"
//#include "Fact.h"
//#include "QGCLoggingCategory.h"
#include "QmlObjectListModel.h"

class SurveyMissionItem;
class SimpleMissionItem;
class MissionController;
#ifdef UNITTEST_BUILD
class MissionItemTest;
#endif

// Represents a Mavlink mission command.
class MissionItem : public QObject
{
    Q_OBJECT
public:
    MissionItem(QObject* parent = NULL);
    MissionItem(int                 sequenceNumber,
                MAV_CMD             command,
                MAV_FRAME           frame,
                const QVariant      &param1,
                const QVariant      &param2,
                const QVariant      &param3,
                const QVariant      &param4,
                const QVariant      &param5,
                const QVariant      &param6,
                const QVariant      &param7,
                bool                autoContinue=true,
                bool                isCurrentItem=false,
                MAV_MISSION_TYPE    tp = MAV_MISSION_TYPE_MISSION,
                QObject*            parent = NULL);
    MissionItem(const MissionItem& other, QObject* parent = NULL);
    ~MissionItem();

    MissionItem &operator=(const MissionItem& other);
	MAV_CMD         command(void)const;
	bool            isCurrentItem(void)const;
	int             sequenceNumber(void)const;
	MAV_FRAME       frame(void)const;
	bool            autoContinue(void)const;
	QVariant        param1(void)const;
	QVariant        param2(void)const;
	QVariant        param3(void)const;
	QVariant        param4(void)const;
	QVariant        param5(void)const;
	QVariant        param6(void)const;
	QVariant        param7(void)const;
	QGeoCoordinate  coordinate() const;
    int             doJumpId() const;
    MAV_MISSION_TYPE missionType()const;
    /// @return Flight speed change value if this item supports it. If not it returns NaN.
    double specifiedFlightSpeed(void) const;
    /// @return Flight gimbal yaw change value if this item supports it. If not it returns NaN.
    double specifiedGimbalYaw(void) const;
    void setCommand(MAV_CMD command);
    void setSequenceNumber(int sequenceNumber);
    void setIsCurrentItem(bool isCurrentItem);
    void setFrame(MAV_FRAME frame);
    void setAutoContinue(bool autoContinue);
    void setParam1(const QVariant  &param1);
    void setParam2(const QVariant  &param2);
    void setParam3(const QVariant  &param3);
    void setParam4(const QVariant  &param4);
    void setParam5(const QVariant  &param5);
    void setParam6(const QVariant  &param6);
    void setParam7(const QVariant  &param7);
    void setCoordinate(const QGeoCoordinate& coordinate);
    bool relativeAltitude(void) const;
signals:
    void isCurrentItemChanged       (bool isCurrentItem);
    void sequenceNumberChanged      (int sequenceNumber);
    void specifiedFlightSpeedChanged(double flightSpeed);
    void specifiedGimbalYawChanged  (double gimbalYaw);
private slots:
    void _param2Changed         (QVariant value);
    void _param3Changed         (QVariant value);
private:
    bool _convertJsonV1ToV2(const QJsonObject& json, QJsonObject& v2Json, QString& errorString);
    bool _convertJsonV2ToV3(QJsonObject& json, QString& errorString);

    int				    m_sequenceNumber;
    int				    m_doJumpId;
    bool			    m_isCurrentItem;
	bool			    m_autoContinue;
	MAV_CMD             m_command;
	MAV_FRAME           m_frame;
    QVariant            m_params[7];
    MAV_MISSION_TYPE    m_missionType;
};

#endif
