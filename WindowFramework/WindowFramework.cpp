#include "stdafx.h"

#include "WindowFramework.h"
#include "WindowImpl.h"

const wchar_t CLASS_NAME[] = L"Window Framework Class";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WindowImpl* window = reinterpret_cast<WindowImpl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    
    switch(uMsg)
    {
        case WM_CLOSE:
            window->OnClose();
            break;
        default:
            break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

class WindowFrameworkImpl : public WindowFramework
{
public:
    virtual void Init() override;

    virtual void Shutdown() override;

    virtual void StartUpdate(const double delta) override {}

    virtual void FinishUpdate() override;

    virtual std::unique_ptr<Window> CreateWindow(const std::string& name, const Window* parent, const std::function<void()>& OnClose) override;
    virtual HINSTANCE GetInstance() { return m_hInstance; }

private:
    HINSTANCE m_hInstance;
};

void WindowFrameworkImpl::Init()
{
    m_hInstance = GetModuleHandle(nullptr);
    Assert(m_hInstance);

    WNDCLASS wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);
}

void WindowFrameworkImpl::Shutdown()
{
    UnregisterClass(CLASS_NAME, m_hInstance);
}

void WindowFrameworkImpl::FinishUpdate()
{
    MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        DispatchMessage(&msg);
    }
}

std::unique_ptr<Window> WindowFrameworkImpl::CreateWindow(const std::string& name, const Window* parent, const std::function<void()>& OnClose)
{
    std::unique_ptr<WindowImpl> ret = std::make_unique<WindowImpl>(OnClose);
    ret->Create(CLASS_NAME, name, m_hInstance, static_cast<const WindowImpl*>(parent));
    return ret;
}


std::unique_ptr<WindowFramework> WindowFramework::Create()
{
    return std::make_unique<WindowFrameworkImpl>();
}
