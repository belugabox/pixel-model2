/* Minimal miniz_export.h to satisfy include in vendorized miniz.h */
#pragma once

#if defined(_WIN32) && defined(MINIZ_DLL)
#  ifdef MINIZ_IMPLEMENTATION
#    define MINIZ_EXPORT __declspec(dllexport)
#  else
#    define MINIZ_EXPORT __declspec(dllimport)
#  endif
#else
#  define MINIZ_EXPORT
#endif

/* This header intentionally does not redeclare miniz typedefs; those live in miniz.h */
