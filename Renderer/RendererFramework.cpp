#include "stdafx.h"
#include "RendererFramework.h"

#include <WindowFramework/WindowFramework.h>
#include <WindowFramework/Window.h>

#include <limits>
#include <fstream>

class RendererFrameworkImpl : public RendererFramework
{
public:
    RendererFrameworkImpl(WindowFramework& window_framework) : m_window_framework(window_framework) {}
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void StartUpdate(const double delta) override {}
    virtual void FinishUpdate() override {}
    virtual bool ShouldExit() override { return !m_window; }

private:
    void OnMainWindowClose();

    void SetupVKInstance();
    void SetupVKPhysicalDevice();
    void SetupVKDevice();
    void SetupVKCommandPool();
    void SetupVKCommandQueue();
    void SetupVKSurface();
    void SetupVKSwapchain();
    void SetupVKImageViews();
    void SetupVKDepthBuffer();
    void SetupVKUniformBuffer();
    void SetupVKDescriptors();
    void SetupVKPipeline();
    void SetupVKDescriptorPool();
    void SetupDescriptorSets();
    void SetupRenderPass();
    void SetupShaders();

    WindowFramework& m_window_framework;

    std::unique_ptr<Window> m_window{};

    vk::Instance m_vk_instance{};
    vk::PhysicalDevice m_vk_physical_device{};
    vk::Device m_vk_device{};
    vk::CommandPool m_vk_command_pool{};
    vk::CommandBuffer m_vk_command_buffer{};
    vk::Extent2D m_vk_extent{};
    vk::SurfaceKHR m_vk_surface{};
    vk::Format m_vk_format{};
    vk::SwapchainKHR m_vk_swapchain{};

    struct ImageBuffer
    {
        //same index
        std::vector<vk::Image> images{};
        std::vector<vk::ImageView> image_views{};
    } m_image_buffer{};

    struct DepthBuffer
    {
        vk::Image image{};
        vk::ImageView image_view{};
        vk::DeviceMemory memory{};
    } m_depth_buffer{};

    struct UniformBuffer
    {
        vk::Buffer buffer{};
        vk::DeviceMemory memory{};
    } m_uniform_buffer{};

    vk::DescriptorSetLayout m_vk_descriptor_set_layout{};
    vk::PipelineLayout m_vk_pipeline_layout{};
    vk::DescriptorPool m_vk_descriptor_pool{};
    vk::DescriptorSet m_vk_descriptor_set{};
    vk::RenderPass m_vk_render_pass{};
    vk::ShaderModule m_vk_vertex_shader_module{};
    vk::ShaderModule m_vk_fragment_shader_module{};
};

template<typename T>
static T Get(vk::ResultValue<T>&& res)
{
    Assert(res.result == vk::Result::eSuccess);
    return res.value;
}

void RendererFrameworkImpl::Init()
{
    // Create a window
    m_window = m_window_framework.CreateWindow("Game", nullptr, std::bind(&RendererFrameworkImpl::OnMainWindowClose, this));
    Assert(m_window);
    m_window->Show();

    // Vulkan stuff
    SetupVKInstance();
    SetupVKPhysicalDevice();
    SetupVKDevice();
    SetupVKCommandPool();
    SetupVKCommandQueue();
    SetupVKSurface();
    SetupVKSwapchain();
    SetupVKImageViews();
    SetupVKDepthBuffer();
    SetupVKUniformBuffer();
    SetupVKDescriptors();
    SetupVKPipeline();
    SetupVKDescriptorPool();
    SetupDescriptorSets();
    SetupRenderPass();
    SetupShaders();
}

void RendererFrameworkImpl::Shutdown()
{
}

void RendererFrameworkImpl::OnMainWindowClose()
{
    m_window.release();
}

void RendererFrameworkImpl::SetupVKInstance()
{
    const vk::ApplicationInfo app_info;

    const char* instance_extensions[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };

    const vk::InstanceCreateInfo inst_info({}, &app_info, 0, nullptr, static_cast<uint32_t>(countof(instance_extensions)), instance_extensions);
    m_vk_instance = Get(vk::createInstance(inst_info));
}

void RendererFrameworkImpl::SetupVKPhysicalDevice()
{
    Assert(m_vk_instance);
    const auto& physical_devices = Get(m_vk_instance.enumeratePhysicalDevices());
    Assert(!physical_devices.empty());
    m_vk_physical_device = physical_devices[0];
}

void RendererFrameworkImpl::SetupVKDevice()
{
    Assert(m_vk_physical_device);
    const char* device_extensions[] = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    const vk::DeviceCreateInfo device_info({}, 0, nullptr, 0, 0, static_cast<uint32_t>(countof(device_extensions)), device_extensions);
    m_vk_device = Get(m_vk_physical_device.createDevice(device_info));
}

void RendererFrameworkImpl::SetupVKCommandPool()
{
    Assert(m_vk_device);
    const vk::CommandPoolCreateInfo command_pool_info;
    m_vk_command_pool = Get(m_vk_device.createCommandPool(command_pool_info));
}

void RendererFrameworkImpl::SetupVKCommandQueue()
{
    Assert(m_vk_device);
    const vk::CommandBufferAllocateInfo command_buffer_info(m_vk_command_pool, vk::CommandBufferLevel::ePrimary, 1);
    const auto& allocated_command_buffers = Get(m_vk_device.allocateCommandBuffers(command_buffer_info));
    Assert(allocated_command_buffers.size() == 1);
    m_vk_command_buffer = allocated_command_buffers[0];
}

void RendererFrameworkImpl::SetupVKSurface()
{
    Assert(m_window);
    Assert(m_vk_instance);
    const vk::Win32SurfaceCreateInfoKHR surface_create_info({}, m_window_framework.GetInstance(), m_window->GetHandle());
    m_vk_surface = Get(m_vk_instance.createWin32SurfaceKHR(surface_create_info));
}

void RendererFrameworkImpl::SetupVKSwapchain()
{
    Assert(m_vk_physical_device);
    Assert(m_vk_surface);
    Assert(m_vk_device);

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

    const auto& surface_formats = Get(m_vk_physical_device.getSurfaceFormatsKHR(m_vk_surface));
    Assert(!surface_formats.empty());

    m_vk_format = vk::Format::eB8G8R8A8Unorm;
    if(surface_formats[0].format != vk::Format::eUndefined)
    {
        m_vk_format = surface_formats[0].format;
    }

    const auto& surface_capabilities = Get(m_vk_physical_device.getSurfaceCapabilitiesKHR(m_vk_surface));
    const auto& surface_present_modes = Get(m_vk_physical_device.getSurfacePresentModesKHR(m_vk_surface));

    m_vk_extent = surface_capabilities.currentExtent;
    if(m_vk_extent.width == 0xFFFFFFFF)
    {
        const auto& window_size = m_window->GetSize();
        const vk::Extent2D window_extent = {window_size.first, window_size.second};
        m_vk_extent = 
        {
            std::clamp(window_extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width),
            std::clamp(window_extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height)
        };
    }

    vk::SurfaceTransformFlagBitsKHR pre_transform = surface_capabilities.currentTransform;
    if(surface_capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
    {
        pre_transform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
    }

    vk::SwapchainCreateInfoKHR swapchain_info
    (
        {}, //flags
        m_vk_surface, //surface
        surface_capabilities.minImageCount, //minImageCount
        m_vk_format, //image format
        vk::ColorSpaceKHR::eSrgbNonlinear, //image color space
        m_vk_extent, //image extent
        1, //image array layers
        vk::ImageUsageFlagBits::eColorAttachment, //image usage flags
        vk::SharingMode::eExclusive, //image sharing mode
        0, //queue family index count
        nullptr, //queue family indices
        pre_transform, //pre transform
        vk::CompositeAlphaFlagBitsKHR::eOpaque, //composite alphas
        vk::PresentModeKHR::eFifo, //present mode
        0, //clipped
        {} //old swapchain
    );

    if(graphics_queue_family_index != present_queue_family_index)
    {
        const uint32_t queueFamilyIndices[] = { graphics_queue_family_index, present_queue_family_index };
        swapchain_info.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices = queueFamilyIndices;
    }
    m_vk_swapchain = Get(m_vk_device.createSwapchainKHR(swapchain_info));
}

void RendererFrameworkImpl::SetupVKImageViews()
{
    Assert(m_vk_device);
    Assert(m_vk_swapchain);
    
    m_image_buffer.images = Get(m_vk_device.getSwapchainImagesKHR(m_vk_swapchain));

    const size_t num_swapchain_images = m_image_buffer.images.size();
    m_image_buffer.image_views.resize(num_swapchain_images);

    for(uint32_t i = 0; i < num_swapchain_images; ++i)
    {
        vk::ImageViewCreateInfo image_view_info
        (
            {}, 
            m_image_buffer.images[i], 
            vk::ImageViewType::e2D, 
            m_vk_format, 
            vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA),
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
        );

        m_image_buffer.image_views[i] = Get(m_vk_device.createImageView(image_view_info));
    }
}

void RendererFrameworkImpl::SetupVKDepthBuffer()
{
    Assert(m_vk_physical_device);
    Assert(m_vk_device);

    const vk::Format depth_format = vk::Format::eD16Unorm;

    const auto& depth_props = m_vk_physical_device.getFormatProperties(depth_format);

    vk::ImageTiling image_tiling;
    if(depth_props.optimalTilingFeatures)
    {
        image_tiling = vk::ImageTiling::eOptimal;
    }
    else if(depth_props.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
    {
        image_tiling = vk::ImageTiling::eLinear;
    }
    else
    {
        std::abort(); //TODO add message
    }

    const vk::ImageCreateInfo image_info
    (
        {}, 
        vk::ImageType::e2D,
        depth_format,
        {m_vk_extent.width, m_vk_extent.height, 1},
        1,
        1,
        vk::SampleCountFlagBits::e16,
        image_tiling,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::SharingMode::eExclusive,
        0,
        nullptr,
        vk::ImageLayout::eUndefined
    );
    m_depth_buffer.image = Get(m_vk_device.createImage(image_info));

    const auto& mem_reqs = m_vk_device.getImageMemoryRequirements(m_depth_buffer.image);
    const auto& mem_properties = m_vk_physical_device.getMemoryProperties();

    const vk::MemoryPropertyFlags mem_flags = vk::MemoryPropertyFlagBits::eDeviceLocal;
    uint32_t memory_type_index;
    for(memory_type_index = 0; memory_type_index < mem_properties.memoryTypeCount; ++memory_type_index)
    {
        if 
        (
            ((mem_properties.memoryTypes[memory_type_index].propertyFlags & mem_flags) == mem_flags) //has type flag
            && ((mem_reqs.memoryTypeBits >> memory_type_index) & 1) //mem_reqs accepts that index
        )
        {
            break;
        }
    }
    Assert(memory_type_index < mem_properties.memoryTypeCount);

    const vk::MemoryAllocateInfo alloc_info(mem_reqs.size, memory_type_index);
    m_depth_buffer.memory = Get(m_vk_device.allocateMemory(alloc_info));
    m_vk_device.bindImageMemory(m_depth_buffer.image, m_depth_buffer.memory, 0);

    const vk::ImageViewCreateInfo image_view_info
    (
        {}, 
        m_depth_buffer.image, 
        vk::ImageViewType::e2D, 
        depth_format, 
        vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA),
        vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1)
    );

    m_depth_buffer.image_view = Get(m_vk_device.createImageView(image_view_info));
}

void RendererFrameworkImpl::SetupVKUniformBuffer()
{
    Assert(m_vk_physical_device);
    Assert(m_vk_device);

    const vk::BufferCreateInfo buffer_info
    (
        {},
        sizeof(glm::mat4), //TODO something more appropriate
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::SharingMode::eExclusive
    );

    m_uniform_buffer.buffer = Get(m_vk_device.createBuffer(buffer_info));

    const auto& mem_reqs = m_vk_device.getBufferMemoryRequirements(m_uniform_buffer.buffer);
    const auto& mem_properties = m_vk_physical_device.getMemoryProperties();

    const vk::MemoryPropertyFlags mem_flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    uint32_t memory_type_index;
    for(memory_type_index = 0; memory_type_index < mem_properties.memoryTypeCount; ++memory_type_index)
    {
        if 
        (
            ((mem_properties.memoryTypes[memory_type_index].propertyFlags & mem_flags) == mem_flags) //has type flag
            && ((mem_reqs.memoryTypeBits >> memory_type_index) & 1) //mem_reqs accepts that index
        )
        {
            break;
        }
    }
    Assert(memory_type_index < mem_properties.memoryTypeCount);

    const vk::MemoryAllocateInfo alloc_info(mem_reqs.size, memory_type_index);
    m_uniform_buffer.memory = Get(m_vk_device.allocateMemory(alloc_info));

    void* mem = Get(m_vk_device.mapMemory(m_uniform_buffer.memory, 0, sizeof(glm::mat4), {}));
    glm::mat4 identity;
    memcpy(mem, &identity, sizeof(glm::mat4));
    m_vk_device.unmapMemory(m_uniform_buffer.memory);
}

void RendererFrameworkImpl::SetupVKDescriptors()
{
    Assert(m_vk_device);

    const vk::DescriptorSetLayoutBinding layout_binding
    (
        0, 
        vk::DescriptorType::eUniformBuffer, 
        1, 
        vk::ShaderStageFlagBits::eVertex
    );

    const vk::DescriptorSetLayoutCreateInfo layout_create_info
    (
        {},
        1,
        &layout_binding
    );

    m_vk_descriptor_set_layout = Get(m_vk_device.createDescriptorSetLayout(layout_create_info));
}

void RendererFrameworkImpl::SetupVKPipeline()
{
    Assert(m_vk_device);
    Assert(m_vk_descriptor_set_layout);

    const vk::PipelineLayoutCreateInfo layout_create_info
    (
        {},
        1,
        &m_vk_descriptor_set_layout,
        0,
        nullptr
    );

    m_vk_pipeline_layout = Get(m_vk_device.createPipelineLayout(layout_create_info));
}

void RendererFrameworkImpl::SetupVKDescriptorPool()
{
    Assert(m_vk_device);

    const vk::DescriptorPoolSize pool_size
    (
        vk::DescriptorType::eUniformBuffer,
        1
    );

    const vk::DescriptorPoolCreateInfo pool_create_info
    (
        {},
        1,
        1,
        &pool_size
    );

    m_vk_descriptor_pool = Get(m_vk_device.createDescriptorPool(pool_create_info));
}

void RendererFrameworkImpl::SetupDescriptorSets()
{
    Assert(m_vk_device);
    Assert(m_vk_descriptor_set_layout);
    Assert(m_vk_descriptor_pool);
    Assert(m_uniform_buffer.buffer);

    const vk::DescriptorSetAllocateInfo allocate_info
    (
        m_vk_descriptor_pool,
        1,
        &m_vk_descriptor_set_layout
    );
    auto sets = Get(m_vk_device.allocateDescriptorSets(allocate_info));
    Assert(sets.size() == 1);
    m_vk_descriptor_set = sets[0];

    const vk::DescriptorBufferInfo descriptor_buffer_info(m_uniform_buffer.buffer, 0, 0);

    const vk::WriteDescriptorSet write
    (
        m_vk_descriptor_set,
        0,
        0,
        1,
        vk::DescriptorType::eUniformBuffer,
        nullptr,
        &descriptor_buffer_info
    );

    m_vk_device.updateDescriptorSets(1, &write, 0, nullptr);
}

void RendererFrameworkImpl::SetupRenderPass()
{
    Assert(m_vk_device);

    const auto num_samples = vk::SampleCountFlagBits::e16;

    const vk::AttachmentDescription attachments[2] =
    {
        vk::AttachmentDescription
        (
            {},
            m_vk_format,
            num_samples,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::ePresentSrcKHR
        ),
        vk::AttachmentDescription
        (
            {},
            m_vk_format,
            num_samples,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eDontCare,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthStencilAttachmentOptimal
        )
    };

    const vk::AttachmentReference color_reference(0, vk::ImageLayout::eColorAttachmentOptimal);
    const vk::AttachmentReference depth_reference(0, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    const vk::SubpassDescription subpass_description
    (
        {},
        vk::PipelineBindPoint::eGraphics,
        0,
        nullptr,
        1,
        &color_reference,
        nullptr,
        &depth_reference,
        0,
        nullptr
    );
    
    const vk::RenderPassCreateInfo render_pass_create_info
    (
        {},
        2,
        attachments,
        1,
        &subpass_description,
        0,
        nullptr
    );

    m_vk_render_pass = Get(m_vk_device.createRenderPass(render_pass_create_info));
}

void RendererFrameworkImpl::SetupShaders()
{
    Assert(m_vk_device);

    auto ReadByteCode = [](auto& filename)
    {
        std::vector<uint32_t> ret;
        std::ifstream file(filename, std::ifstream::in | std::ifstream::binary);
        Assert(file);
        
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();

        Assert
        (
            (size > 0)
            && ((size % sizeof(uint32_t)) == 0)
            && ((size / sizeof(uint32_t)) <= std::numeric_limits<uint32_t>::max())
        );
        ret.resize(static_cast<size_t>(size / sizeof(uint32_t)));

        file.seekg(std::ios::beg);
        file.read(reinterpret_cast<char*>(&ret[0]), size);

        return ret;
    };

    std::vector<uint32_t> vertex_shader_bytecode = ReadByteCode("./Resources/Shaders/Simple.vert.spv");
	const vk::ShaderModuleCreateInfo vertex_shader_module_create_info
	(
		{},
        vertex_shader_bytecode.size(),
        &vertex_shader_bytecode[0]
	);
    m_vk_vertex_shader_module = Get(m_vk_device.createShaderModule(vertex_shader_module_create_info));

    std::vector<uint32_t> fragment_shader_bytecode = ReadByteCode("./Resources/Shaders/Simple.frag.spv");
    const vk::ShaderModuleCreateInfo fragment_shader_module_create_info
    (
        {},
        fragment_shader_bytecode.size(),
        &fragment_shader_bytecode[0]
    );
    m_vk_fragment_shader_module = Get(m_vk_device.createShaderModule(fragment_shader_module_create_info));
}

std::unique_ptr<RendererFramework> RendererFramework::Create(WindowFramework& window_framework)
{
    return std::make_unique<RendererFrameworkImpl>(window_framework);
}
