#pragma once

#include "DllExport.h"

#include <Base/Framework.h>
#include <Base/WindowsInclude.h>
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
    virtual HINSTANCE GetInstance() = 0;
};
