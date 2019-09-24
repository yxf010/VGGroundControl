#include "VGOperationInformation.h"

VGOperationInformation::VGOperationInformation(QObject *parent) : QObject(parent)
{
    m_serviceCharge = 0;
    m_pesticideCharge = 0;
    m_otherCharge = 0;
    m_sumCharge = 0;
    m_bSelected = false;
}

VGOperationInformation::VGOperationInformation(VGOperationInformation *info)
{
    m_serviceCharge = info->serviceCharge();
    m_bSelected = info->m_bSelected;
    m_actualDesId = info->m_actualDesId;
    m_operatorId = info->m_operatorId;
    m_registerId = info->m_registerId;
    m_chargerId = info->m_chargerId;
    m_beginTime = info->m_beginTime;
    m_endTime = info->m_endTime;
    m_operationType = info->m_operationType;
    m_actualOperationId = info->m_actualOperationId;
    m_notes = info->m_notes;
    m_deviceId = info->m_deviceId;
    m_deviceName = info->m_deviceName;
    m_pesticideId = info->m_pesticideId;
    m_pesticideName = info->m_pesticideName;
    m_cropName = info->m_cropName;
    m_cropId = info->m_cropId;
    m_pesticideCharge = info->m_pesticideCharge;
    m_otherCharge = info->m_otherCharge;
    m_sumCharge = info->m_sumCharge;
}
