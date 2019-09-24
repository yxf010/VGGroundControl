/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/
#include "base.h"
#include <qmath.h>
#include <QDateTime>

quint64 BASE::groundTimeUsecs()
{
    return groundTimeMilliseconds() * 1000;
}

quint64 BASE::groundTimeMilliseconds()
{
    return static_cast<quint64>(QDateTime::currentMSecsSinceEpoch());
}

qreal BASE::groundTimeSeconds()
{
    return static_cast<qreal>(groundTimeMilliseconds()) / 1000.0f;
}

double BASE::toMirrorAngle(double angle)
{
    if (angle > -20 * M_PI && angle < 20 * M_PI)
    {
        int n = angle * 2 / M_PI;
        if (n % 2)
            return angle - (n / 2)*M_PI;

        return angle > 0 ? angle - ((n + 1) / 2)*M_PI : angle - ((n - 1) / 2)*M_PI;
    }

    return angle;
}
///////////////////////////////////////////////////////////////////////////
//Application
///////////////////////////////////////////////////////////////////////////
Application * Application::s_instance = NULL;
Application::Application()
{
    s_instance = this;
}


Application * Application::Instance()
{
    return s_instance;
}
