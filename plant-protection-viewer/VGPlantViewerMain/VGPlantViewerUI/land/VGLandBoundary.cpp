#include "VGLandBoundary.h"
#include "MapItemFactory.h"
#include "VGFlyRoute.h"
#include "RouteStruct.h"
#include "VGApplication.h"
#include "VGMapManager.h"
#include "VGCoordinate.h"
#include "VGOutline.h"
#include "VGLandPolygon.h"
#include "VGLandManager.h"
#include "VGPlanningWorker.h"
#include "VGGlobalFun.h"
#include <QThread>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//VGLandBoundary
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VGLandBoundary::VGLandBoundary(QObject *info) : SingleTriggerItem<MapAbstractItem>(qobject_cast<VGLandInformation *>(info))
, m_bSaveLocal(false), m_safeArea(NULL)
, m_bUploaded(false), m_time(0)
, m_area(0), m_curBlock(-1)
, m_bSafeEdited(false)
{
    m_bShowBelonged = false;
}

VGLandBoundary::VGLandBoundary(const VGLandBoundary &oth) : SingleTriggerItem<MapAbstractItem>(oth)
, m_bSaveLocal(oth.m_bSaveLocal), m_safeArea(NULL)
, m_bUploaded(oth.m_bUploaded), m_bSafeEdited(true)
, m_curBlock(-1), m_area(oth.m_area), m_time(oth.m_time)
, m_strDescribe(oth.m_strDescribe), m_cloudID(oth.m_cloudID)
{
    SetShowBelonged(oth.IsShowBelonged());
    if (oth.m_safeArea)
    {
        m_safeArea = new VGLandPolygon(*oth.m_safeArea);
        m_safeArea->SetParentItem(this);
        m_safeArea->SetVisible(GetVisible());
    }

    foreach(VGLandPolygon *itr, oth.m_polygons)
    {
        VGLandPolygon *item = new VGLandPolygon(*itr);
        item->SetParentItem(this);
        item->SetVisible(GetVisible());
        m_polygons << item;
        connect(item, &VGLandPolygon::pathChanged, this, &VGLandBoundary::onBoundaryChange);
        if (item->GetId() == VGLandPolygon::Boundary)
            m_center = VGPlanningWorker::GetPolygonCenter(*item);
    }
    _calcArea();
}

VGLandBoundary::~VGLandBoundary()
{
    if (VGLandInformation *info = GetBelongedLand())
        info->deleteLater();
}

VGLandInformation *VGLandBoundary::GetBelongedLand() const
{
    return qobject_cast<VGLandInformation *>(GetParentItem());
}

void VGLandBoundary::SetBelongedLand(VGLandInformation *land)
{
    SetParentItem(land);
    if (land)
        land->InsertBoundary(this);
}

MapAbstractItem::MapItemType VGLandBoundary::ItemType() const
{
    return MapAbstractItem::Type_LandBoundary;
}

int VGLandBoundary::BlocksCount() const
{
    int ret = 0;
    foreach (VGLandPolygon *itr, m_polygons)
    {
        if (itr->GetId() == VGLandPolygon::BlockBoundary)
            ++ret;
    }
    return ret;
}

QString VGLandBoundary::GetSurveyDescribe() const
{
    VGLandInformation::SurveyType tp = VGLandInformation::Survey_No;
    if (VGLandInformation *land = GetBelongedLand())
        tp = land->GetSurveyType();

    switch (tp)
    {
    case MapAbstractItem::Survey_DrawMap:
        return QString::fromStdWString(L"手动标点");
    case MapAbstractItem::Survey_GPS:
        return QString::fromStdWString(L"GPS踩点");
    case MapAbstractItem::Survey_Vehicle:
        return QString::fromStdWString(L"测绘设备踩点");
    default:
        return QString::fromStdWString(L"未知");;
    }
}

QString VGLandBoundary::GetUser() const
{
    if (VGLandInformation *land = GetBelongedLand())
        return land->userId();

    return QString();
}

QString VGLandBoundary::GetOwnerName() const
{
    if (VGLandInformation *land = GetBelongedLand())
        return land->ownerName();

    return QString();
}

QString VGLandBoundary::GetOwnerPhone() const
{
    if (VGLandInformation *land = GetBelongedLand())
        return land->ownerPhone();

    return QString();
}

double VGLandBoundary::GetLandArea() const
{
    return m_area;
}

int VGLandBoundary::GetPrecision() const
{
    if (VGLandInformation *land = GetBelongedLand())
        return land->precision();

    return 1;
}

QString VGLandBoundary::GetBoundaryId() const
{
    return m_cloudID;
}

VGLandInformation::OwnerStruct *VGLandBoundary::owner()
{
    if (VGLandInformation *lan = GetBelongedLand())
        return lan->owner();

    return NULL;
}

QString VGLandBoundary::GetLandActId() const
{
    if (VGLandInformation *lan = GetBelongedLand())
        return lan->actualSurveyId();

    return QString();
}

void VGLandBoundary::SetBoundaryId(const QString &str)
{
    if (m_cloudID == str)
        return;

    m_cloudID = str;
    emit boundaryIdChanged(str);
}

void VGLandBoundary::showContent(bool b)
{
    if (m_safeArea)
        m_safeArea->Show(b);

    foreach(VGLandPolygon *itr, m_polygons)
    {
        itr->Show(b);
    }

    if (b && m_parentItem)
        m_parentItem->showContent(IsShowBelonged());
}

void VGLandBoundary::SetShowType(int t)
{
    foreach(VGLandPolygon *itr, m_polygons)
    {
        itr->SetShowType(t);
    }
}

void VGLandBoundary::addBoudaryPoint(const QGeoCoordinate &coor, VGCoordinate *c)
{
    VGLandPolygon *bdr = GetBoundaryPolygon();
    if (!bdr)
        bdr = _addBoundary(VGLandPolygon::Boundary);

    if (!bdr || !coor.isValid())
        return;

    foreach(VGCoordinate *co, bdr->GetCoordinates())
    {
        double tmp = co->DistanceToCoor(coor);
        if (tmp < 4 || tmp > 2000)
        {
            qvgApp->SetQmlTip(QString::fromStdWString(L"边界点在4-2000米之间"), true);
            return;
        }
    }
    bdr->AddCoordinate(coor, VGCoordinate::Boundary, c);
}

void VGLandBoundary::addBlockPoint(const QGeoCoordinate &coor, VGCoordinate *c)
{
    if (!coor.isValid())
        return;

    if (VGLandPolygon *block = _getCurBlock())
    {
        foreach(VGCoordinate *co, block->GetCoordinates())
        {
            double tmp = co->DistanceTo(coor);
            if (tmp < 2 || tmp > 100)
            {
                qvgApp->SetQmlTip(QString::fromStdWString(L"障碍物在2-100米之间"), true);
                return;
            }
        }
        block->AddCoordinate(coor, VGCoordinate::BlockBoundary, c);
        return;
    }
    qvgApp->SetQmlTip(QString::fromStdWString(L"请先添加障碍物"), true);
}

void VGLandBoundary::addBlock()
{
    if (!_isLastBlockValid())
        return;

    if (_addBoundary(VGLandPolygon::BlockBoundary))
    {
        SetCurBlockIndex(BlocksCount() - 1);
        emit countBlocksChanged();
        emit validChanged(false);
    }
}

void VGLandBoundary::clearBoudary()
{
    if (VGLandPolygon *bdr = GetBoundaryPolygon())
        bdr->Clear();
}

void VGLandBoundary::clearSurvey()
{
    if (VGLandInformation *land = GetBelongedLand())
        land->clearCoors();
}

VGLandBoundary *VGLandBoundary::adjust()
{
    VGLandManager *mgr = qvgApp->landManager();
    VGLandInformation *land = mgr->Clone(GetBelongedLand(), true);
    VGLandBoundary *ret = mgr ? (VGLandBoundary *)Clone(land) : NULL;
    if (ret)
    {
        ret->SetBelongedLand(land);
        ret->m_time = 0;
        if (land)
        {
            land->setActualSurveyId(QString());
            land->SetSaveLocal(false);
            land->SetUploaded(false);
        }
        if (VGMapManager *mgr = qvgApp->mapManager())
        {
            mgr->InsertItem(land);
            mgr->InsertItem(ret);
        }

        mgr->AddChild(ret);
        ret->m_bSaveLocal = false;
        ret->m_bUploaded = false;
        ret->SetShowType(MapAbstractItem::Show_LineAndPoint);
        ret->SetSelected(true);
    }
    return ret;
}

VGLandPolygon *VGLandBoundary::_addBoundary(int tp)
{
    if (tp != VGLandPolygon::Boundary && tp != VGLandPolygon::BlockBoundary)
        return NULL;

    if (VGLandPolygon *tmp = new VGLandPolygon(this, tp))
    {
        tmp->Show(GetVisible());
        m_polygons << tmp;
        connect(tmp, &VGLandPolygon::pathChanged, this, &VGLandBoundary::onBoundaryChange);
        connect(tmp, &VGLandPolygon::countCoorChanged, this, &VGLandBoundary::onBoundaryCoorsChanged);
        return tmp;
    }
    return NULL;
}

void VGLandBoundary::_addSafeAreaPoint(const QGeoCoordinate &coor)
{
    if (m_safeArea && coor.isValid())
        m_safeArea->AddCoordinate(coor, VGCoordinate::Boundary);
}

VGLandManager *VGLandBoundary::GetLandManager() const
{
    return qvgApp->landManager();
}

MapAbstractItem::SurveyType VGLandBoundary::GetSurveyType() const
{
    if (VGLandInformation *land = GetBelongedLand())
        return land->GetSurveyType();

    return MapAbstractItem::Survey_No;
}

void VGLandBoundary::processSaveReslt(const DescribeMap &result)
{
    DescribeMap::const_iterator itr = result.find(BoundaryId);
    if (itr != result.end())
    {
        QString actId = itr.value().toString();
        if (actId != m_cloudID && !actId.isEmpty())
        {
            if (itr != result.end())
            {
                QString actId = itr.value().toString();
                if (actId != m_cloudID && !actId.isEmpty())
                {
                    SetBoundaryId(actId);
                    SetUploaded(true);
					UpdateDB();
                }
            }
        }
    }

    if (VGLandInformation *bdr = GetBelongedLand())
    {
        itr = result.find(LandID);
        if (itr != result.end())
        {
            bdr->setActualSurveyId(itr.value().toString());
            bdr->SetUploaded(true);
            bdr->UpdateDB();
        }
    }
}

MapAbstractItem *VGLandBoundary::FindParentByProperty(const DescribeMap &dsc) const
{
    DescribeMap dscParent = VGGlobalFunc::ParentDescribe(dsc, *this);
    for (VGLandInformation *land : qvgApp->landManager()->Lands())
    {
        DescribeMap tmp;
        VGGlobalFunc::initialItemDescribe(tmp, *land, dsc.contains("actualId"));
        if (MapAbstractItem::IsContain(dscParent, tmp))
            return land;
    }
    return NULL;
}

void VGLandBoundary::releaseSafe()
{
    MapAbstractItem::releaseSafe();
    if (VGLandInformation *land = GetBelongedLand())
        land->SetRemoveDB();

    if (VGLandManager *mgr = qvgApp->landManager())
    {
        foreach (VGFlyRoute *itr, mgr->GetRoutesOfBoundary(this))
        {
            itr->releaseSafe();
        }
    }
}

bool VGLandBoundary::IsValid() const
{
    return GetBelongedLand() && _isBoundaryEdited() && _isLastBlockValid();
}

int VGLandBoundary::GetCurBlockIndex() const
{
    return m_curBlock;
}

void VGLandBoundary::SetCurBlockIndex(int i)
{
    if (i == m_curBlock)
        return;

    m_curBlock = i;
    emit curBlockChanged(i);
}

qint64 VGLandBoundary::GetTime() const
{
    return m_time;
}

MapAbstractItem *VGLandBoundary::Clone(QObject *parent) const
{
    if (VGLandBoundary *ret = new VGLandBoundary(*this))
    {
        if (MapAbstractItem *tmp = dynamic_cast<MapAbstractItem*>(parent))
            ret->SetParentItem(tmp);
        else
            ret->setParent(parent);

        ret->SetBoundaryId(QString());
        return ret;
    }
    return NULL;
}

const QList<VGLandPolygon*> & VGLandBoundary::Polygos() const
{
    return m_polygons;
}

QString VGLandBoundary::GetAddress() const
{
    VGLandInformation *land = GetBelongedLand();
    return land ? land->GetAddress() : QString();
}

void VGLandBoundary::SetAddress(const QString &str)
{
    if (VGLandInformation *land = GetBelongedLand())
    {
        land->SetAddress(str);
        emit addressChanged(str);
    }
}

QString VGLandBoundary::GetStatDescribe() const
{
    return m_strStatDescribe;
}

void VGLandBoundary::SetStatDescribe(const QString &str)
{
    m_strStatDescribe = str;
}

void VGLandBoundary::SetSelected(bool b)
{
    SingleTriggerItem<MapAbstractItem>::SetSelected(b);
    if (!b)
        return;

    if (VGLandManager *mgr = GetLandManager())
        mgr->BoundaryChanged(GetBoundarys());
}

QGeoCoordinate VGLandBoundary::GetCenter() const
{
    return m_center;
}

bool VGLandBoundary::IsSaveLocal() const
{
	return m_bSaveLocal;
}

void VGLandBoundary::SetSaveLocal(bool bSl)
{
	if (bSl == m_bSaveLocal)
		return;

	m_bSaveLocal = bSl;
	emit saveLocalChanged(bSl);
}

bool VGLandBoundary::IsUpLoaded() const
{
	return m_bUploaded;
}

void VGLandBoundary::SetUploaded(bool bUpload)
{
	if (bUpload == m_bUploaded)
		return;

	m_bUploaded = bUpload;
	SetSaveLocal(false);
	emit uplaodedChanged(bUpload);
}

bool VGLandBoundary::_isBoundaryEdited() const
{
    if (VGLandPolygon *plg = GetBoundaryPolygon())
        return plg->CountCoordinate() > 2;

    return false;
}

VGLandPolygon *VGLandBoundary::GetBoundaryPolygon()const
{
    foreach(VGLandPolygon *itr, m_polygons)
    {
        if (itr->GetId() == VGLandPolygon::Boundary)
            return itr;
    }

    return NULL;
}

QString VGLandBoundary::GetDescribe() const
{
    return m_strDescribe;
}

void VGLandBoundary::SetDescribe(const QString &str)
{
    if (str.isEmpty())
        return;

    m_strDescribe = str;
    emit describeChanged(str);
}

void VGLandBoundary::onPlanRouteFinished()
{
}

void VGLandBoundary::onBoundaryChange()
{
    _calcArea();
    int idx = m_polygons.indexOf((VGLandPolygon*)sender());
    if (idx < 0)
        return;

    VGLandPolygon *plg = m_polygons.at(idx);
    if (plg->GetId() == VGLandPolygon::Boundary)
    {
        m_center = VGPlanningWorker::GetPolygonCenter(*plg);
        emit centerChanged(m_center);
        bool bValid = plg && plg->CountCoordinate() > 2 && GetBelongedLand();
        emit validChanged(bValid);
    }
    else if (plg->GetId() == VGLandPolygon::BlockBoundary)
    {
        if (plg->CountCoordinate() == 0)
        {
            m_polygons.removeAll(plg);
            if (m_curBlock == idx)
                SetCurBlockIndex(idx - 1);
            plg->deleteLater();
            emit countBlocksChanged();
        }
    }
}

void VGLandBoundary::onBoundaryCoorsChanged(int count)
{
    if (count == 2 || count == 3)
        emit validChanged(IsValid());
}

bool VGLandBoundary::save(bool bCloud)
{
    if (!bCloud)
    {
        if (!IsSaveLocal())
        {
            if(m_time==0)
            {
                m_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
                emit editTimeChanged();
            }
            emit editFinished(this);
        }
        if (VGLandInformation *land = GetBelongedLand())
        {
            if (!land->IsSaveLocal())
                land->save(bCloud);
        }
    }
    return MapAbstractItem::save(bCloud);
}

void VGLandBoundary::_calcArea()
{
    double area = 0;
    foreach(VGLandPolygon *plg, m_polygons)
    {
        if (plg->GetId() == VGLandPolygon::Boundary)
            area += plg->CalculateArea();
        else if (plg->GetId() == VGLandPolygon::BlockBoundary)
            area -= plg->CalculateArea();
    }
    m_area = area * 3 / 2000;
    emit landAreaChanged(m_area);
}

bool VGLandBoundary::_isLastBlockValid()const
{
    foreach(VGLandPolygon *itr, m_polygons)
    {
        if (itr->GetId() == VGLandPolygon::BlockBoundary && !itr->IsValid())
            return false;
    }
    return true;
}

const QList<double> & VGLandBoundary::GetBoundarys()
{
    if (m_boundarys.count() == 4 || !GetBoundaryPolygon())
        return m_boundarys;

    foreach(const QVariant &itr, GetBoundaryPolygon()->path())
    {
        QGeoCoordinate coor = itr.value<QGeoCoordinate>();
        if (coor.isValid())
            VGMapManager::CalcBoundaryByCoor(m_boundarys, coor);
    }
    return m_boundarys;
}

VGLandPolygon *VGLandBoundary::_getCurBlock() const
{
    if (m_curBlock >= 0 && m_curBlock < BlocksCount())
    {
        int idx = 0;
        foreach(VGLandPolygon *itr, m_polygons)
        {
            if (itr->GetId() == VGLandPolygon::BlockBoundary)
            {
                if (idx++ == m_curBlock)
                    return itr;
            }
        }
    }
    return NULL;
}

bool VGLandBoundary::operator==(const MapAbstractItem &item) const
{
    if (item.ItemType() != ItemType())
        return false;

    const VGLandBoundary &bound = *(VGLandBoundary*)&item;
    if (*m_safeArea == *bound.m_safeArea)
        return false;
    if (m_safeArea && *m_safeArea != *bound.m_safeArea)
        return false;

    return true;
}

DECLARE_ITEM_FACTORY(VGLandBoundary)
