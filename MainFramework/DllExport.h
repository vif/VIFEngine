#pragma once

#if defined(_WIN32) && defined(DLL)
#  ifdef MainFramework_EXPORTS
#    define MainFrameWorkEXPORT __declspec(dllexport)
#  else
#    define MainFrameWorkEXPORT __declspec(dllimport)
#  endif
#  else
#endif

#ifndef MainFrameWorkEXPORT
#  define MainFrameWorkEXPORT
#endif