#pragma once

#if defined(_WIN32) && defined(DLL)
#  ifdef Base_EXPORTS
#    define BaseEXPORT __declspec(dllexport)
#  else
#    define BaseEXPORT __declspec(dllimport)
#  endif
#  else
#endif

#ifndef BaseEXPORT
#  define BaseEXPORT
#endif