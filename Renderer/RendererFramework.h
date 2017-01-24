#pragma once

#include "DllExport.h"

#include <Base/Framework.h>

class WindowFramework;

class RendererEXPORT RendererFramework : public Framework
{
public:
    static std::unique_ptr<RendererFramework> Create(WindowFramework& window_framework);

    virtual void Init() override = 0;
    virtual void Shutdown() override = 0;
    virtual void StartUpdate(const double delta) override = 0;
    virtual void FinishUpdate() override = 0;
};

