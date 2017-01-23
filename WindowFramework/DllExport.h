#pragma once

#if defined(_WIN32) && defined(DLL)
#  ifdef WindowFramework_EXPORTS
#    define WindowFrameworkEXPORT __declspec(dllexport)
#  else
#    define WindowFrameworkEXPORT __declspec(dllimport)
#  endif
#  else
#endif

#ifndef WindowFrameworkEXPORT
#  define WindowFrameworkEXPORT
#endif
