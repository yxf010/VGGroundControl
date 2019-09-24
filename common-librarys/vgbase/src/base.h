#ifndef BASE_H
#define BASE_H

#include "vgbase_global.h"
namespace BASE
{
    enum MyEnum
    {
        defaultComponentId = 0,
        defaultSystemId = 255,
    };
    /*** @brief Get the current ground time in microseconds.*/
    quint64 VGBASESHARED_EXPORT groundTimeUsecs();
    /** @brief Get the current ground time in milliseconds */
    quint64 VGBASESHARED_EXPORT groundTimeMilliseconds();
    /** @brief Get the current ground time in fractional seconds */
    qreal VGBASESHARED_EXPORT groundTimeSeconds();
    /** @brief Returns the angle limited to -pi - pi */
    double VGBASESHARED_EXPORT toMirrorAngle(double angle);
}

class QSettings;
class VGBASESHARED_EXPORT Application
{
public:
    virtual QSettings *GetSettings()const = 0;
    virtual QString GetDefualtPath() = 0;
    virtual bool IsPcApp()const = 0;
public:
    static Application *Instance();
protected:
    Application();
    virtual ~Application(){}
private:
    static Application *s_instance;
};

#define QGC_EVENTLOOP_DEBUG 0

#endif // QGC_H
