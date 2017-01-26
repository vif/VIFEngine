#pragma once

#include "DllExport.h"

#include <Base/Framework.h>

class WindowFramework;

class RendererEXPORT RendererFramework : public Framework
{
public:
    static std::unique_ptr<RendererFramework> Create(WindowFramework& window_framework);
};

