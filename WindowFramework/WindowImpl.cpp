#include "stdafx.h"
#include "WindowImpl.h"

void WindowImpl::Create
(
    const wchar_t* class_name,
    const std::string& name, 
    const HINSTANCE& hInstance, 
    const WindowImpl* parent
)
{
    m_HWND = CreateWindowEx
    (
        0,                                 // Optional window styles.
        class_name,                        // Window class
        ToUTF16(name).c_str(),             // Window text
        WS_OVERLAPPEDWINDOW,               // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        parent ? parent->m_HWND : nullptr, // Parent window    
        nullptr,                           // Menu
        hInstance,                         // Instance handle
        nullptr                            // Additional application data
    );
    Assert(m_HWND);
}

void WindowImpl::Show()
{
    ShowWindow(m_HWND, true);
}

void WindowImpl::Hide()
{
    ShowWindow(m_HWND, false);
}

HWND WindowImpl::GetHandle()
{
    return m_HWND;
}
