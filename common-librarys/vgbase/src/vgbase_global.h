#ifndef VGBASE_GLOBAL_H
#define VGBASE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(VGBASE_LIBRARY)
#  define VGBASESHARED_EXPORT Q_DECL_EXPORT
#elif defined(VGBASE_LIBRARY_SL)
#  define VGBASESHARED_EXPORT
#else
#  define VGBASESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // VGBASE_GLOBAL_H
