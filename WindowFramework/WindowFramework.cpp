#include "stdafx.h"

#include "WindowFramework.h"
#include "Window.h"

const wchar_t CLASS_NAME[] = L"Window Framework Class";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

class WindowFrameworkImpl : public WindowFramework
{
public:
    virtual void Init() override 
    {
        m_hInstance = GetModuleHandle(nullptr);
        Assert(m_hInstance);

        WNDCLASS wc{};
        wc.lpfnWndProc   = WindowProc;
        wc.hInstance     = m_hInstance;
        wc.lpszClassName = CLASS_NAME;
        RegisterClass(&wc);
    }

    virtual void Shutdown() override {}
    virtual void StartUpdate(const double delta) override {}

    virtual void FinishUpdate() override
    {
        MSG msg;
        BOOL bRet;
        while((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
        { 
            if(bRet == -1)
            {
                // handle the error and possibly exit
            }
            else
            {
                TranslateMessage(&msg); 
                DispatchMessage(&msg); 
            }
        } 
    }

    virtual std::unique_ptr<Window> CreateWindow(const std::string& name, const Window* parent) override;

private:
    HINSTANCE m_hInstance;
};

class WindowImpl : public Window
{
public:
    WindowImpl() = default;

    void Create(const std::string& name, const HINSTANCE& hInstance,const WindowImpl* parent)
    {
        m_HWND = CreateWindowEx
        (
            0,                                 // Optional window styles.
            CLASS_NAME,                                // Window class
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

    virtual void Show() override
    {
        ShowWindow(m_HWND, true);
    }
    
    virtual void Hide() override
    {
        ShowWindow(m_HWND, false);
    }

private:
    HWND m_HWND;
};

std::unique_ptr<Window> WindowFrameworkImpl::CreateWindow(const std::string& name, const Window* parent)
{
    std::unique_ptr<WindowImpl> ret = std::make_unique<WindowImpl>();
    ret->Create(name, m_hInstance, static_cast<const WindowImpl*>(parent));
    return ret;
}


std::unique_ptr<WindowFramework> WindowFramework::Create()
{
    return std::make_unique<WindowFrameworkImpl>();
}
