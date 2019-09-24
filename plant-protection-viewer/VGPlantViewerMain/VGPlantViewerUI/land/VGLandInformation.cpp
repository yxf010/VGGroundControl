#include "VGLandInformation.h"
#include <QDebug>
#include <QDateTime>

#include "MapItemFactory.h"
#include "VGApplication.h"
#include "VGMapManager.h"
#include "VGLandBoundary.h"
#include "VGCoordinate.h"
#include "VGOutline.h"
#include "VGLandPolygon.h"
#include "VGLandManager.h"
#include "VGGetGpsLocation.h"
#include "VGSurveyMonitor.h"
#include "VGToolBox.h"

#define DOUBLELIIM 1000000
#define ISEQDouble(f1, f2) (int)((f1)*DOUBLELIIM)==(int)((f2)*DOUBLELIIM)
///////////////////////////////////////////////////////////////////////////////////
//VGLandInformation
///////////////////////////////////////////////////////////////////////////////////
VGLandInformation::VGLandInformation(QObject *parent) : SingleTriggerItem<MapAbstractItem>(parent, 1)
, m_bUploaded(false), m_bSaveLocal(false), m_typeSurvey(Survey_No), m_bSurvey(true)
, m_bRemoveDB(false), m_time(0), m_precision(1)
{                        
    qRegisterMetaType<QGeoCoordinate>("QGeoCoordinate");
    setUserId(qvgApp->GetUserId());
}

VGLandInformation::VGLandInformation(const VGLandInformation &info) : SingleTriggerItem<MapAbstractItem>(info.parent(), 1)
, m_bUploaded(info.m_bUploaded), m_bSaveLocal(info.m_bSaveLocal)
, m_userId(info.m_userId), m_actualSurveyId(info.m_actualSurveyId)
, m_time(info.m_time), m_owner(info.m_owner), m_bSurvey(false)
, m_precision(info.m_precision), m_bRemoveDB(false)
, m_adress(info.m_adress), m_typeSurvey(info.m_typeSurvey)
{
	SetItemColor(QColor(255 * 0.4, 255 * 0.9, 255 * 0.4, 127), QColor(255, 127, 127, 127));
    foreach (VGCoordinate *itr, info.m_coorsSurvey)
    {
        _addOneBoundaryPoint(itr->GetCoordinate(), itr->GetId());
    }
}

VGLandInformation::~VGLandInformation()
{
    qDeleteAll(m_coorsSurvey);
}

MapAbstractItem *VGLandInformation::Clone(QObject *parent) const
{
    if (VGLandInformation *ret = new VGLandInformation(*this))
    {
        ret->setParent(parent);
        ret->setActualSurveyId(QString());
        return ret;
    }
    return NULL;
}

void VGLandInformation::SetSelected(bool b)
{
    if (GetSelected()!=b)
        MapAbstractItem::SetSelected(b);
    else if (b)
        Show(true);
}

void VGLandInformation::InsertBoundary(VGLandBoundary *bdy, int idx)
{
    if (bdy && m_boundarys.indexOf(bdy) < 0)
    {
        m_bRemoveDB = false;
        if (idx <0 || idx > m_boundarys.count())
            m_boundarys.append(bdy);
        else
            m_boundarys.insert(idx, bdy);

        connect(bdy, &VGLandBoundary::destroyed, this, &VGLandInformation::onContentDestroyed);
    }
}

void VGLandInformation::SetRemoveDB()
{
    if (m_boundarys.count()<=1)
        m_bRemoveDB = true;
}

void VGLandInformation::clearCoors()
{
    for(VGCoordinate *co : m_coorsSurvey)
    {
        if (co->GetId() != VGCoordinate::Home)
            co->deleteLater();
    }
}

bool VGLandInformation::save(bool bCloud)
{
    if (m_typeSurvey == Survey_DrawMap)
        disconnect(VGLandManager::GetGps(), &VGGetGpsLocation::sigGetGpsCoordinate, this, &VGLandInformation::sltGetGpsCoordinate);

    if (m_time == 0)
        m_time = QDateTime::currentMSecsSinceEpoch();

    return MapAbstractItem::save(bCloud);
}

VGLandInformation::OwnerStruct * VGLandInformation::owner()
{
    return &m_owner;
}

QGeoCoordinate VGLandInformation::GetCheckCoord() const
{
    for (VGCoordinate *itr : m_coorsSurvey)
    {
        if (itr->GetId() == VGCoordinate::Home)
            return itr->GetCoordinate();
    }

    return QGeoCoordinate();
}

void VGLandInformation::SetCheckCoord(const QGeoCoordinate &c)
{
    if (c.isValid())
    {
        for (VGCoordinate *itr : m_coorsSurvey)
        {
            if (itr->GetId() == VGCoordinate::Home)
                return itr->SetCoordinate(c);
        }

        _addOneBoundaryPoint(c, VGCoordinate::Home);
    }
}

void VGLandInformation::setOwnerName(const QString &param)
{
    if (param.isEmpty() || param == m_owner.strName)
        return;

    m_owner.strName = param;
    emit ownerNameChanged(param);
}

qint64 VGLandInformation::GetOwnerBirthday() const
{
    return m_owner.tmBirthday;
}

QString VGLandInformation::ownerBirthday() const
{
    return QDateTime::fromMSecsSinceEpoch(m_owner.tmBirthday).toString("yyyy-MM-dd");
}

void VGLandInformation::setOwnerBirthday(const QString &param)
{
    qint64 nMsec = QDateTime::fromString(param, "yyyy-MM-dd").toMSecsSinceEpoch();
    if (m_owner.tmBirthday == nMsec)
        return; 

    m_owner.tmBirthday = nMsec;
    emit ownerBirthdayChanged(param);
}

void VGLandInformation::setOwnerAddr(const QString &param)
{
    if (param.isEmpty() || param == m_owner.strAddress)
        return;

    m_owner.strAddress = param;
    emit ownerAddrChanged(param);
}

void VGLandInformation::setOwnerPhone(const QString &param)
{
    if (param.isEmpty() || param == m_owner.strPhone)
        return;

    m_owner.strPhone = param;
    emit ownerPhoneChanged(param);
}

QString VGLandInformation::GetAddress() const
{
    return m_adress;
}

void VGLandInformation::SetAddress(const QString &addr)
{
    if (addr.isEmpty() || addr == m_adress)
        return;

    m_adress = addr;
    emit addressChanged(addr);
}

void VGLandInformation::setUserId(const QString &user)
{
    if (user.isEmpty() || user == m_userId)
        return;

    m_userId = user;
    emit userIdChanged(user);
}

void VGLandInformation::SetSurveyTime(qint64 tm)
{
    if (m_time == tm)
        return;

    m_time = tm;
    emit surveyTimeChanged(tm);
}

void VGLandInformation::setPrecision(int precision)
{
    if (precision == m_precision)
        return;

    m_precision = precision;
    emit precisionChanged(precision);
}

void VGLandInformation::SetSaveLocal(bool bSl)
{
    if (bSl == m_bSaveLocal)
        return;

    m_bSaveLocal = bSl;
    emit saveLocalChanged(bSl);
}

void VGLandInformation::SetUploaded(bool bUpload)
{
    if (bUpload == m_bUploaded)
        return;

    m_bUploaded = bUpload;
    SetSaveLocal(false);
    emit uplaodedChanged(bUpload);
}

QString VGLandInformation::actualSurveyId() const
{
    return m_actualSurveyId;
}

void VGLandInformation::setActualSurveyId(QString id)
{
    if (m_actualSurveyId == id)
        return;

    m_actualSurveyId = id;
    emit actualSurveyIdIdChanged(id);
}

MapAbstractItem::MapItemType VGLandInformation::ItemType() const
{
    return MapAbstractItem::Type_LandInfo;
}

void VGLandInformation::showContent(bool b)
{
    foreach (VGCoordinate *itr, m_coorsSurvey)
    {
        itr->Show(b);
    }
}

bool VGLandInformation::IsSurveyLand() const
{
    if (Survey_No <= m_typeSurvey || m_typeSurvey > Survey_Vehicle)
        return false;

    if (m_typeSurvey == Survey_DrawMap)
        return m_bSurvey;
    if (m_typeSurvey == Survey_GPS)
        return m_bSurvey && qvgApp->landManager()->isGPSOpened();
    if (m_typeSurvey == Survey_Vehicle)
        return m_bSurvey && qvgApp->toolbox()->surveyVehicle();

	return false;
}

void VGLandInformation::SetSurveyLand(bool b)
{
    if (b || b == m_bSurvey)
        return;

    m_bSurvey = false;
    emit surveyChanged(false);
}

void VGLandInformation::SetSurveyType(SurveyType tp)
{
    if (tp > Survey_Vehicle || tp < Survey_No)
        return;

    m_typeSurvey = tp;
    if (tp == Survey_DrawMap)
    {
        VGLandManager *mgr = qvgApp->landManager();
        if (mgr && !mgr->isGPSOpened())
            mgr->startGetGps(30);

        if (VGGetGpsLocation *gps = VGLandManager::GetGps())
            connect(gps, &VGGetGpsLocation::sigGetGpsCoordinate, this, &VGLandInformation::sltGetGpsCoordinate);
    }
    else if (VGSurveyMonitor *mnt = qvgApp->surveyMonitor())
    {
        SetCheckCoord(mnt->GetCheckCoord());
    }

    emit surveyTypeChanged(tp);
}

VGLandInformation::SurveyType VGLandInformation::GetSurveyType() const
{
    return m_typeSurvey;
}

void VGLandInformation::sltGetGpsCoordinate(double lat, double lon, double alt, int sig)
{
    if (m_typeSurvey == Survey_DrawMap)
        _addOneBoundaryPoint(QGeoCoordinate(lat, lon, alt), sig);
}

void VGLandInformation::onContentDestroyed(QObject *obj)
{
    int idx = m_coorsSurvey.indexOf((VGCoordinate*)obj);
    if (idx >= 0)
    {
        m_coorsSurvey.removeAt(idx);
    }
    else if ((idx = m_boundarys.indexOf((VGLandBoundary*)obj)) >= 0)
    {
        m_boundarys.removeAt(idx);
        if (m_boundarys.count() == 0)
        {
            if (m_bRemoveDB)
                releaseSafe();
            else
                deleteLater();
        }
    }
}

void VGLandInformation::_addOneBoundaryPoint(const QGeoCoordinate &coor, int nSat)
{
    if (!coor.isValid())
        return;

    foreach(VGCoordinate *co, m_coorsSurvey)
    {
        if (co->DistanceTo(coor) < 1)
            return;
    }

    if (VGCoordinate *co = new VGCoordinate(coor, nSat, this))
    {
        m_coorsSurvey << co;
        connect(co, &VGCoordinate::destroyed, this, &VGLandInformation::onContentDestroyed);
        co->Show(GetVisible());
    }
}

void VGLandInformation::releaseSafe()
{
    SingleTriggerItem<MapAbstractItem>::releaseSafe();
    foreach(VGLandBoundary *itr, m_boundarys)
    {
        itr->releaseSafe();
    }
}

bool VGLandInformation::operator==(const MapAbstractItem &item)const
{
    if (item.ItemType() != ItemType())
        return false;

    const VGLandInformation &li = *static_cast<const VGLandInformation*>(&item);

    if (li.m_actualSurveyId != m_actualSurveyId)
        return false;
    if (li.m_userId != m_userId)
        return false;
    if (li.m_time != m_time)
        return false;
    if (li.m_adress != m_adress)
        return false;
    if (li.ownerName() != ownerName())
        return false;
    if (li.ownerAddr() != ownerAddr())
        return false;
    if (li.ownerPhone() != ownerPhone())
        return false;
    if (li.ownerBirthday() != ownerBirthday())
        return false;
    if (li.m_precision != m_precision)
        return false;
    if (m_coorsSurvey.count() != li.m_coorsSurvey.count())
        return false;

    QList<VGCoordinate*>::const_iterator itr = m_coorsSurvey.begin();
    QList<VGCoordinate*>::const_iterator itr2 = li.m_coorsSurvey.begin();
    for (; itr != m_coorsSurvey.end() && itr2 != li.m_coorsSurvey.end(); ++itr, ++itr2)
    {
        if (**itr != **itr2)
            return false;
    }

    return true;
}
///////////////////////////////////////////////////////////////////////
//OwnerStruct
///////////////////////////////////////////////////////////////////////
VGLandInformation::OwnerStruct::OwnerStruct()
:tmBirthday(QDateTime::currentMSecsSinceEpoch())
{
}

VGLandInformation::OwnerStruct::OwnerStruct(const OwnerStruct &oth)
    : strName(oth.strName), strAddress(oth.strAddress)
    , tmBirthday(oth.tmBirthday)
    , strPhone(oth.strPhone)
{
}

QString VGLandInformation::OwnerStruct::ToString() const
{
    return QString("%1;%2;%3;%4").arg(strName).arg(strAddress)
        .arg(QDateTime::fromMSecsSinceEpoch(tmBirthday).toString("yyyy-MM-dd")).arg(strPhone);
}

void VGLandInformation::OwnerStruct::ParseString(const QString &owner)
{
    QStringList strLs = owner.split(";");
    if (strLs.count())
        strName = strLs.takeFirst();
    if (strLs.count())
        strAddress = strLs.takeFirst();
    if (strLs.count())
        tmBirthday = QDateTime::fromString(strLs.takeFirst(), "yyyy-MM-dd").toMSecsSinceEpoch();
    if (strLs.count())
        strPhone = strLs.takeFirst();
}

DECLARE_ITEM_FACTORY(VGLandInformation)
