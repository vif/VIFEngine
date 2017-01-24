#include "stdafx.h"
#include "RendererFramework.h"

#include <WindowFramework/WindowFramework.h>
#include <WindowFramework/Window.h>

class RendererFrameworkImpl : public RendererFramework
{
public:
    RendererFrameworkImpl(WindowFramework& window_framework) : m_window_framework(window_framework) {}
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void StartUpdate(const double delta) override {}
    virtual void FinishUpdate() override {}

private:
    WindowFramework& m_window_framework;

    std::unique_ptr<Window> m_window;

    vk::Instance m_vk_inst;
    vk::Device m_vk_device;
    vk::CommandPool m_vk_command_pool;
    vk::CommandBuffer m_vk_command_buffer;
    vk::SurfaceKHR m_surface;
};

void RendererFrameworkImpl::Init()
{
    vk::ApplicationInfo app_info;

    const char* instance_extensions[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };

    vk::InstanceCreateInfo inst_info{ vk::InstanceCreateFlags(), &app_info, 0, nullptr, static_cast<uint32_t>(countof(instance_extensions)), instance_extensions};
    const auto& create_instance_res = vk::createInstance(inst_info);
    Assert(create_instance_res.result == vk::Result::eSuccess);
    m_vk_inst = create_instance_res.value;

    const auto& enumerate_res = m_vk_inst.enumeratePhysicalDevices();
    Assert(enumerate_res.result == vk::Result::eSuccess);
    Assert(!enumerate_res.value.empty());
    const auto& physical_device = enumerate_res.value[0];

    const char* device_extensions[] = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    vk::DeviceCreateInfo device_info(vk::DeviceCreateFlags(), 0, nullptr, 0, 0, static_cast<uint32_t>(countof(device_extensions)), device_extensions);
    const auto& create_device_res = physical_device.createDevice(device_info);
    Assert(create_device_res.result == vk::Result::eSuccess);
    m_vk_device = create_device_res.value;
    
    vk::CommandPoolCreateInfo command_pool_info;
    const auto& create_pool_res = m_vk_device.createCommandPool(command_pool_info);
    Assert(create_pool_res.result == vk::Result::eSuccess);
    m_vk_command_pool = create_pool_res.value;

    vk::CommandBufferAllocateInfo command_buffer_info{ m_vk_command_pool, vk::CommandBufferLevel::ePrimary, 1 };
    const auto& allo_commandbuff_res = m_vk_device.allocateCommandBuffers(command_buffer_info);
    Assert(allo_commandbuff_res.result == vk::Result::eSuccess);
    Assert(allo_commandbuff_res.value.size() == 1);
    m_vk_command_buffer = allo_commandbuff_res.value[0];

    m_window = m_window_framework.CreateWindow("Game", nullptr);
    Assert(m_window);
    m_window->Show();

    vk::Win32SurfaceCreateInfoKHR surface_create_info{ vk::Win32SurfaceCreateFlagsKHR(), m_window_framework.GetInstance(), m_window->GetHandle() };
    const auto& create_surface_res = m_vk_inst.createWin32SurfaceKHR(surface_create_info);
    Assert(create_surface_res.result == vk::Result::eSuccess);
    m_surface = create_surface_res.value;
}

void RendererFrameworkImpl::Shutdown()
{
    m_vk_device.destroy();
    m_vk_inst.destroy();
}

std::unique_ptr<RendererFramework> RendererFramework::Create(WindowFramework& window_framework)
{
    return std::make_unique<RendererFrameworkImpl>(window_framework);
}