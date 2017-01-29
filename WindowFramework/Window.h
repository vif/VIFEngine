#pragma once

#include "DllExport.h"

#include <Base/WindowsInclude.h>

class WindowFrameworkEXPORT Window
{
public:
    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual HWND GetHandle() = 0;
    virtual std::pair<uint32_t, uint32_t> GetSize() = 0;
};
