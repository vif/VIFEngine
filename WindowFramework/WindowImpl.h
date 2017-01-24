#pragma once

#include "Window.h"

#include <string.h>

class WindowImpl : public Window
{
public:
    WindowImpl() = default;

    virtual void Create
    (
        const wchar_t* class_name, 
        const std::string& name, 
        const HINSTANCE& hInstance, 
        const WindowImpl* parent
    );
    virtual void Show() override;
    virtual void Hide() override;
    virtual HWND GetHandle() override;

private:
    HWND m_HWND;
};