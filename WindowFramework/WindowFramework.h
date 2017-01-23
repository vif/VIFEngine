#pragma once

#include <Base/Framework.h>

#include "DllExport.h"

#include <string>

//responsible for creating & managing Windows

class Window;

class WindowFrameworkEXPORT WindowFramework : public Framework
{
public:
    static std::unique_ptr<WindowFramework> Create();

    virtual void Init() override = 0;
    virtual void Shutdown() override = 0;
    virtual void StartUpdate(const double delta) override = 0;
    virtual void FinishUpdate() override = 0;

    virtual std::unique_ptr<Window> CreateWindow(const std::string& name, const Window* parent) = 0;
};
