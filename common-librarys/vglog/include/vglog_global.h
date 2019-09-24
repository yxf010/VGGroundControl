#ifndef VGLOG_GLOBAL_H
#define VGLOG_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(VGLOG_LIBRARY)
#  define VGLOGSHARED_EXPORT Q_DECL_EXPORT
#elif defined(VGLOG_LIBRARY_SL)
#  define VGLOGSHARED_EXPORT
#else
#  define VGLOGSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // VGLOG_GLOBAL_H
