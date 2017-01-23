#pragma once

#include "DllExport.h"

#define _HAS_EXCEPTIONS 0

#include <string>
#include <vector>

BaseEXPORT std::string ToUTF8(const std::wstring& str);
BaseEXPORT std::wstring ToUTF16(const std::string& str);