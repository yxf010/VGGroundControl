#ifndef __SHAREFUNCTION_H__
#define __SHAREFUNCTION_H__

#include "share_global.h"

#define  DoubleEqu(x, y) ((int)(((x) - (y)) * 0x00100000) == 0)

namespace ShareFunction{
    //check cpu is big or little endian
    SHARED_DECL bool IsBigEndian(void);
    //火星地图转GPS位置转
    SHARED_DECL void gpsCorrect(double &lat, double &lon);
    SHARED_DECL void toGps(double &lat, double &lon);
    //火星地图转百度地图
    SHARED_DECL void gcjTranceBaidu(double &lat, double &lon);
}

#endif // __SHAREFUNCTION_H__

