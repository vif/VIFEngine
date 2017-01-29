#pragma once

#include "Window.h"

class WindowImpl : public Window
{
public:
    WindowImpl(const std::function<void()>& OnClose);

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
    virtual std::pair<uint32_t, uint32_t> GetSize() override;

    void OnClose() { m_OnClose(); }

private:
    HWND m_HWND;
    std::function<void()> m_OnClose;
};