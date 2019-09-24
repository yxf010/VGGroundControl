
#include "logHelper.h"


LogHelper *LogHelper::m_pInstance = NULL;

LogHelper::LogHelper()
{

}

LogHelper *LogHelper::getInstance()
{
    if(m_pInstance == NULL)
    {
        m_pInstance = new LogHelper();
    }
    return m_pInstance;
}

void LogHelper::notifyUI(QString strMessage)
{
    emit sigSendMessage2UI(strMessage);
}
