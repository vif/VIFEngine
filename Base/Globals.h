#pragma once

#include "DllExport.h"

#define _HAS_EXCEPTIONS 0

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

BaseEXPORT std::string ToUTF8(const std::wstring& str);
BaseEXPORT std::wstring ToUTF16(const std::string& str);

#define Assert(expr) (void)(!!(expr) || (std::abort(), true))

#ifdef _DEBUG
#  define DebugAssert(expr) Assert(expr)
#else
#  define DebugAssert(expr)
#endif