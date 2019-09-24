#ifndef __SHARE_GLOBAL_H__
#define __SHARE_GLOBAL_H__

#if defined(_MSC_VER) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  define DECL_EXPORT __declspec(dllexport)
#  define DECL_IMPORT __declspec(dllimport)

#elif defined(__ARMCC__) || defined(__CC_ARM)
/* work-around for missing compiler intrinsics */
#if defined(__ANDROID__) || defined(ANDROID)
#  if defined(__linux__) || defined(__linux)
#    define DECL_EXPORT     __attribute__((visibility("default")))
#    define DECL_IMPORT     __attribute__((visibility("default")))
#  else
#    define DECL_EXPORT     __declspec(dllexport)
#    define DECL_IMPORT     __declspec(dllimport)
#  endif
#endif
#elif defined(VISIBILITY_AVAILABLE)
#  define DECL_EXPORT     __attribute__((visibility("default")))
#  define DECL_IMPORT     __attribute__((visibility("default")))
#elif defined __sharegloabal
#  define DECL_EXPORT __global
#else
#  define DECL_EXPORT
#  define DECL_IMPORT
#endif

#ifdef SHARE_LIBRARY
#  define SHARED_DECL  DECL_EXPORT
#else
#  define SHARED_DECL  DECL_IMPORT
#endif

#endif //__SHARE_GLOBAL_H__
