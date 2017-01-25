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
    vk::PhysicalDevice m_vk_physical_device;
    vk::Device m_vk_device;
    vk::CommandPool m_vk_command_pool;
    vk::CommandBuffer m_vk_command_buffer;
    vk::SurfaceKHR m_vk_surface;
    vk::SwapchainKHR m_vk_swapchain;
};

template<typename T>
static T Get(vk::ResultValue<T>&& res)
{
    Assert(res.result == vk::Result::eSuccess);
    return res.value;
}

void RendererFrameworkImpl::Init()
{
    // Create a windows
    m_window = m_window_framework.CreateWindow("Game", nullptr);
    Assert(m_window);
    m_window->Show();

    // Vulkan stuff
    vk::ApplicationInfo app_info;

    const char* instance_extensions[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };

    vk::InstanceCreateInfo inst_info{{}, &app_info, 0, nullptr, static_cast<uint32_t>(countof(instance_extensions)), instance_extensions};
    m_vk_inst = Get(vk::createInstance(inst_info));

    const auto& physical_devices = Get(m_vk_inst.enumeratePhysicalDevices());
    Assert(!physical_devices.empty());
    m_vk_physical_device = physical_devices[0];

    const char* device_extensions[] = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    vk::DeviceCreateInfo device_info({}, 0, nullptr, 0, 0, static_cast<uint32_t>(countof(device_extensions)), device_extensions);
    m_vk_device = Get(m_vk_physical_device.createDevice(device_info));
    
    vk::CommandPoolCreateInfo command_pool_info;
    m_vk_command_pool = Get(m_vk_device.createCommandPool(command_pool_info));

    vk::CommandBufferAllocateInfo command_buffer_info{m_vk_command_pool, vk::CommandBufferLevel::ePrimary, 1};
    const auto& allocated_command_buffers = Get(m_vk_device.allocateCommandBuffers(command_buffer_info));
    Assert(allocated_command_buffers.size() == 1);
    m_vk_command_buffer = allocated_command_buffers[0];

    vk::Win32SurfaceCreateInfoKHR surface_create_info{{}, m_window_framework.GetInstance(), m_window->GetHandle()};
    m_vk_surface = Get(m_vk_inst.createWin32SurfaceKHR(surface_create_info));

    const auto& queue_family_properties = m_vk_physical_device.getQueueFamilyProperties();
    Assert(!queue_family_properties.empty());
    
    uint32_t graphics_queue_family_index = UINT32_MAX;
    uint32_t present_queue_family_index = UINT32_MAX;

    for(uint32_t i = 0; i < queue_family_properties.size(); ++i)
    {
        if((queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics) && (Get(m_vk_physical_device.getSurfaceSupportKHR(i, m_vk_surface))))
        {
            graphics_queue_family_index = i;
            present_queue_family_index = i;
            break;
        }
    }

    if(graphics_queue_family_index == UINT32_MAX)
    {
        for(uint32_t i = 0; i < queue_family_properties.size(); ++i)
        {
            if(queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
            {
                graphics_queue_family_index = i;
                break;
            }
        }

        for(uint32_t i = 0; i < queue_family_properties.size(); ++i)
        {
            if(Get(m_vk_physical_device.getSurfaceSupportKHR(i, m_vk_surface)))
            {
                present_queue_family_index = i;
                break;
            }
        }
    }

    Assert(graphics_queue_family_index != UINT32_MAX);
    Assert(present_queue_family_index != UINT32_MAX);

    const auto& surfaceFormats = Get(m_vk_physical_device.getSurfaceFormatsKHR(m_vk_surface));
    Assert(!surfaceFormats.empty());
    
    vk::Format format = vk::Format::eB8G8R8A8Unorm;
    if(surfaceFormats[0].format != vk::Format::eUndefined)
    {
        format = surfaceFormats[0].format;
    }

    const auto& surface_capabilities = Get(m_vk_physical_device.getSurfaceCapabilitiesKHR(m_vk_surface));
    const auto& surface_present_modes = Get(m_vk_physical_device.getSurfacePresentModesKHR(m_vk_surface));

    vk::Extent2D extent = surface_capabilities.currentExtent;
    if(extent.width == 0xFFFFFFFF) 
    {
        extent = surface_capabilities.maxImageExtent;
    }

    vk::SurfaceTransformFlagBitsKHR preTransform = surface_capabilities.currentTransform;
    if(surface_capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
    {
        preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    }

    vk::SwapchainCreateInfoKHR swapchain_info
    {
        {}, //flags
        m_vk_surface, //surface
        surface_capabilities.minImageCount, //minImageCount
        format, //image format
        vk::ColorSpaceKHR::eSrgbNonlinear, //image color space
        extent, //image extent
        1, //image array layers
        vk::ImageUsageFlagBits::eColorAttachment, //image usage flags
        vk::SharingMode::eExclusive, //image sharing mode
        0, //queue family index count
        nullptr, //queue family indices
        preTransform, //pre transform
        vk::CompositeAlphaFlagBitsKHR::eOpaque, //composite alphas
        vk::PresentModeKHR::eFifo, //present mode
        0, //clipped
        nullptr //old swapchain
    };

    if(graphics_queue_family_index != present_queue_family_index)
    {
        const uint32_t queueFamilyIndices[] = { graphics_queue_family_index, present_queue_family_index };
        swapchain_info.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices = queueFamilyIndices;
    }
    m_vk_swapchain = Get(m_vk_device.createSwapchainKHR(swapchain_info));
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