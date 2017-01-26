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
    virtual std::unique_ptr<Window> CreateWindow(const std::string& name, const Window* parent, const std::function<void()>& OnClose) = 0;
    virtual HINSTANCE GetInstance() = 0;
};
