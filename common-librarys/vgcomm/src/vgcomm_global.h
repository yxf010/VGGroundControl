#ifndef VGCOMM_GLOBAL_H
#define VGCOMM_GLOBAL_H

#include <QtCore/qglobal.h>
//#include "QsLog.h"

#if defined(VGCOMM_LIBRARY)
#  define VGCOMMSHARED_EXPORT Q_DECL_EXPORT
#elif defined(VGCOMM_LIBRARY_SL)
#  define VGCOMMSHARED_EXPORT
#else
#  define VGCOMMSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // VGCOMM_GLOBAL_H
