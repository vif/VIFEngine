#pragma once

#if defined(_WIN32) && defined(DLL)
#  ifdef Renderer_EXPORTS
#    define RendererEXPORT __declspec(dllexport)
#  else
#    define RendererEXPORT __declspec(dllimport)
#  endif
#  else
#endif

#ifndef RendererEXPORT
#  define RendererEXPORT
#endif