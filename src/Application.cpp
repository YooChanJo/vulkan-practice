#include "Application.h"

/* TODO: Remove glm later --> abstraction */
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace VulkanPractice {
    Application::Application(const ApplicationConfig& config)
        : m_ApplicationName(config.ApplicationName), m_ApplicationEngineName(config.ApplicationEngineName)
    {
        if(s_Instance != nullptr) {
            throw std::runtime_error("An Application already exists");
        }
        m_Window = std::make_unique<Window>(config.WindowWidth, config.WindowHeight, config.WindowTitle);
        /* TODO: Move this to window class --> add event handler */
        glfwSetFramebufferSizeCallback(m_Window->GetNativeWindow(), FramebufferResizeCallback);
        Log::Init();
        InitVulkan();
        /* Print Extensions Info */
#ifdef INCLUDE_DEBUG_INFO
            {
                uint32_t extensionCount = 0;
                vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
                std::vector<VkExtensionProperties> extensions(extensionCount);
                vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
                LOG_INFO("Available Vulkan Instance Extensions");
                for (const auto& ext : extensions) {
                    std::cout << "Name: \033[32m" << ext.extensionName 
                    << "\033[0m, Spec version: \033[32m" << ext.specVersion
                    << "\n\033[0m";
                }
            }
            {
                uint32_t layerCount;
                vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
                std::vector<VkLayerProperties> availableLayers(layerCount);
                vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
                LOG_INFO("Available Vulkan Instance Layers");
                for (const auto& layer : availableLayers) {
                    std::cout << "Name: \033[32m" << layer.layerName 
                    << "\033[0m, Spec version: \033[32m" << layer.specVersion
                    << "\033[0m, Implementation version: \033[32m" << layer.implementationVersion
                    << "\n\033[0m"
                    << "\033[0mDescription: \033[32m" << layer.description
                    << "\n\033[0m";
                }
            }
            {
                uint32_t extensionCount;
                vkEnumerateDeviceExtensionProperties(m_VkPhysicalDevice, nullptr, &extensionCount, nullptr);
                std::vector<VkExtensionProperties> extensions(extensionCount);
                vkEnumerateDeviceExtensionProperties(m_VkPhysicalDevice, nullptr, &extensionCount, extensions.data());
                LOG_INFO("Available Physical Device Extensions");
                for (const auto& ext : extensions) {
                    std::cout << "Name: \033[32m" << ext.extensionName 
                    << "\033[0m, Spec version: \033[32m" << ext.specVersion
                    << "\n\033[0m";
                }
            }
#endif
        s_Instance = this;
    }
    /* TODO: Move this to window class --> add event handler */
    void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
        Application::GetInstance()->m_FramebufferResized = true;
    }

    Application::~Application() {
        CleanupVulkan();
        s_Instance = nullptr;
    }
    void Application::Run() {
        while(!glfwWindowShouldClose(m_Window->GetNativeWindow())) {
            DrawFrame();
            glfwPollEvents();
        }
        vkDeviceWaitIdle(m_VkDevice); // wait to finish operations before calling destructor
    }

    void Application::InitVulkan() {
        /* Create Vulkan Instance */
        CreateInstance();
        /* Setup Debug Messenger only for Debug */
#ifdef INCLUDE_DEBUG_INFO
        SetupDebugMessenger();
#endif
        /* Create Surface KHR */
        CreateSurface();
        /* Pick Physical Device */
        PickPhysicalDevice();
        /* Create Logical Device */
        CreateLogicalDevice();
        /* Create Swap Chain */
        CreateSwapChain();
        /* Create Image Views */
        CreateImageViews();
        /* Create Render Pass */
        CreateRenderPass();
        /* Create Graphics Pipeline */
        CreateGraphicsPipeline();
        /* Create Frame Buffers */
        CreateFramebuffers();
        /* Create Command Pool */
        CreateCommandPool();
        /* Create Command Buffer */
        CreateCommandBuffers();
        /* Create Semaphores and Fences */
        CreateSyncObjects();
    }
    void Application::CleanupVulkan() {
        for(size_t i = 0; i < m_MaxFramesInFlight; i++) {
            vkDestroySemaphore(m_VkDevice, m_VkImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(m_VkDevice, m_VkRenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(m_VkDevice, m_VkInFlightFences[i], nullptr);
        }
        vkDestroyCommandPool(m_VkDevice, m_VkCommandPool, nullptr); // automatically frees command buffers
        for (auto framebuffer : m_VkSwapChainFramebuffers) {
            vkDestroyFramebuffer(m_VkDevice, framebuffer, nullptr);
        }
        vkDestroyPipeline(m_VkDevice, m_VkGraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_VkDevice, m_VkPipelineLayout, nullptr);
        vkDestroyRenderPass(m_VkDevice, m_VkRenderPass, nullptr);
        for (auto imageView : m_VkSwapChainImageViews) {
            vkDestroyImageView(m_VkDevice, imageView, nullptr);
        }
        vkDestroySwapchainKHR(m_VkDevice, m_VkSwapchainKHR, nullptr);
        vkDestroyDevice(m_VkDevice, nullptr);
        vkDestroySurfaceKHR(m_VkInstance, m_VkSurfaceKHR, nullptr);
#ifdef INCLUDE_DEBUG_INFO
        DestroyDebugUtilsMessengerEXT(m_VkInstance, m_VkDebugUtilsMessengerEXT, nullptr);
#endif
        vkDestroyInstance(m_VkInstance, nullptr);
    }

    void Application::CreateInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = m_ApplicationName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = m_ApplicationEngineName.c_str();
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        {
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            m_InstanceExtensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef INCLUDE_DEBUG_INFO
            m_InstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
            // If we add app config need to check uniqueness
        }
        if(!CheckInstanceExtensionSupport(m_InstanceExtensions)) {
            throw std::runtime_error("Instance extensions requested, yet not available!");
        }
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_InstanceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_InstanceExtensions.data();

        if(!CheckInstanceLayerSupport(m_InstanceLayers)) {
            throw std::runtime_error("Instance layers requested, yet not available!");
        }
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_InstanceLayers.size());
        createInfo.ppEnabledLayerNames = m_InstanceLayers.data();

        /* Validation Layers for Debug Messenger and Instance */
#ifdef INCLUDE_DEBUG_INFO
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = DebugCallback;
        debugCreateInfo.pUserData = nullptr; // Optional
        debugCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
#endif

        if (vkCreateInstance(&createInfo, nullptr, &m_VkInstance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vkinstance!");
        }
    }
#ifdef INCLUDE_DEBUG_INFO
    void Application::SetupDebugMessenger() {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
        createInfo.pUserData = nullptr; // Optional
        if (CreateDebugUtilsMessengerEXT(m_VkInstance, &createInfo, nullptr, &m_VkDebugUtilsMessengerEXT) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }
#endif
    void Application::CreateSurface() {
        if (glfwCreateWindowSurface(m_VkInstance, m_Window->GetNativeWindow(), nullptr, &m_VkSurfaceKHR) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create window surface!");
        }
    }
    void Application::PickPhysicalDevice() {
        m_VkPhysicalDevice = VK_NULL_HANDLE;
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, devices.data());
        for(const auto& device: devices) {
            if(IsPhysicalDeviceSuitable(device, m_VkSurfaceKHR, m_DeviceExtensions)) {
                m_VkPhysicalDevice = device;
                break; // We are using the first device match, however multiple devices can be used
            }
        }
        if (m_VkPhysicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }
    void Application::CreateLogicalDevice() {
        QueueFamilyIndices indices = FindQueueFamilies(m_VkPhysicalDevice, m_VkSurfaceKHR);
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            indices.GraphicsFamily.value(),
            indices.PresentFamily.value()
        };
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        VkPhysicalDeviceFeatures deviceFeatures{}; // simply define --> enable features for future fancier use

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_DeviceExtensions.data();
        createInfo.enabledLayerCount = 0;
        if (vkCreateDevice(m_VkPhysicalDevice, &createInfo, nullptr, &m_VkDevice) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device!");
        }
        // Get Graphics Queue Handle
        vkGetDeviceQueue(m_VkDevice, indices.GraphicsFamily.value(), 0, &m_VkGraphicsQueue);
        vkGetDeviceQueue(m_VkDevice, indices.PresentFamily.value(), 0, &m_VkPresentQueue);

    }
    void Application::CreateSwapChain() {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_VkPhysicalDevice, m_VkSurfaceKHR);
        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
        VkExtent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities, m_Window);
        uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1; // triple buffering is usually best
        if (swapChainSupport.Capabilities.maxImageCount > 0 && imageCount > swapChainSupport.Capabilities.maxImageCount) {
            imageCount = swapChainSupport.Capabilities.maxImageCount;
        }
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_VkSurfaceKHR;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // for now direct attachment

        QueueFamilyIndices indices = FindQueueFamilies(m_VkPhysicalDevice, m_VkSurfaceKHR);
        uint32_t queueFamilyIndices[] = {
            indices.GraphicsFamily.value(), indices.PresentFamily.value()
        };

        if (indices.GraphicsFamily != indices.PresentFamily) {
            /* Todo: Manage ownership and switch to EXCLUSIVE */
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }
        createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE; // This part meddles when other windows are in front of current window clipping the pixels --> If vulkan is used for compute shaders then might want to set this VK_FALSE
        createInfo.oldSwapchain = VK_NULL_HANDLE; // This is about window resizing --> This would be handled in the future
        if (vkCreateSwapchainKHR(m_VkDevice, &createInfo, nullptr, &m_VkSwapchainKHR) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swap chain!");
        }
        // Get Swapchain images handle
        vkGetSwapchainImagesKHR(m_VkDevice, m_VkSwapchainKHR, &imageCount, nullptr);
        m_SwapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(m_VkDevice, m_VkSwapchainKHR, &imageCount, m_SwapChainImages.data());
        // Get the Formats
        m_VkSwapChainImageFormat = surfaceFormat.format;
        m_VkSwapChainExtent = extent;
    }
    void Application::CreateImageViews() {
        m_VkSwapChainImageViews.resize(m_SwapChainImages.size());
        for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_SwapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_VkSwapChainImageFormat;
            /* Default */
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            // No mulitple layers and no mipmapping
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            if (vkCreateImageView(m_VkDevice, &createInfo, nullptr, &m_VkSwapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image views!");
            }
        }
    }
    /* TODO: Abstract Shader Functionalities */
    static std::vector<char> ReadFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file!");
        }
        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }
    void Application::CreateGraphicsPipeline() {
        /* Shaders */
        auto vertShaderCode = ReadFile(std::string(SHADER_DIR) + "/SPRIV/vert.spv");
        auto fragShaderCode = ReadFile(std::string(SHADER_DIR) + "/SPRIV/frag.spv");
        VkShaderModule vertShaderModule, fragShaderModule;
        {
            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = vertShaderCode.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(vertShaderCode.data());
            if (vkCreateShaderModule(m_VkDevice, &createInfo, nullptr, &vertShaderModule) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create vertex shader module!");
            }
        }
        {
            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = fragShaderCode.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(fragShaderCode.data());
            if (vkCreateShaderModule(m_VkDevice, &createInfo, nullptr, &fragShaderModule) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create fragment shader module!");
            }
        }
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";
        vertShaderStageInfo.pSpecializationInfo = nullptr; // initial values i can set
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";
        fragShaderStageInfo.pSpecializationInfo = nullptr; // initial values i can set
        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        struct Vertex {
            glm::vec2 Position;
            glm::vec3 Color;
            static VkVertexInputBindingDescription getBindingDescription() {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding = 0;
                bindingDescription.stride = sizeof(Vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                return bindingDescription;
            }
        };
        std::vector<Vertex> vertices = {
            {{  0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }},
            {{  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }},
            {{ -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }},
        };

        /* Vertex Input */ // Currently referencing no data
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

        /* Input Assembly */
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE; // splitting

        /* Dynamics States setup requiring these stages to be setup on drawing time not pipeline setup */
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineViewportStateCreateInfo viewportState{}; // since it is dynamic only specify count
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        /* Raserizer */
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE; // might be used for shadows --> Requires GPU Feature
        rasterizer.rasterizerDiscardEnable = VK_FALSE; // if true blocks any output to framebuffer
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // VK_POLYGON_MODE_FILL or VK_POLYGON_MODE_LINE or VK_POLYGON_MODE_POINT possible // any other than fill requires GPU Feature
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

        rasterizer.depthBiasEnable = VK_FALSE; // alter depth values
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        /* Multisampling */
        VkPipelineMultisampleStateCreateInfo multisampling{}; // Allows to store multiple samples per pixel (smoother) --> Requires GPU Feature
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        /* Blending */
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        // // In case of Blending
        // colorBlendAttachment.blendEnable = VK_TRUE;
        // colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        // colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        // colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        // colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        // colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        // colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        /* Regards Uniforms */
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0; // Optional
        pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
        if (vkCreatePipelineLayout(m_VkDevice, &pipelineLayoutInfo, nullptr, &m_VkPipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2; // vertex and fragment
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_VkPipelineLayout;
        pipelineInfo.renderPass = m_VkRenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(m_VkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_VkGraphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(m_VkDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(m_VkDevice, vertShaderModule, nullptr);
    }
    void Application::CreateRenderPass() {
        // Currently only color attachment --> depth testing disabled
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_VkSwapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // regarding multisampling --> currently only one sample per pixel
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear any previous attatchments left
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Keep current stored for later on presenting
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // I don't care what layout it was in before
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // I am going to present to screen

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0; // the location=0 of fragment shader
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        VkRenderPassCreateInfo renderPassInfo{};

        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // previous stage external
        dependency.dstSubpass = 0; // we are configuring the 0th one
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;


        if (vkCreateRenderPass(m_VkDevice, &renderPassInfo, nullptr, &m_VkRenderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        } 
    }
    void Application::CreateFramebuffers() {
        m_VkSwapChainFramebuffers.resize(m_VkSwapChainImageViews.size());
        for (size_t i = 0; i < m_VkSwapChainImageViews.size(); i++) {
            VkImageView attachments[] = {
                m_VkSwapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_VkRenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_VkSwapChainExtent.width;
            framebufferInfo.height = m_VkSwapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_VkDevice, &framebufferInfo, nullptr, &m_VkSwapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffer!");
            }
        }
    }
    void Application::CreateCommandPool() {
        QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_VkPhysicalDevice, m_VkSurfaceKHR);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();
        if (vkCreateCommandPool(m_VkDevice, &poolInfo, nullptr, &m_VkCommandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }
    }
    void Application::CreateCommandBuffers() {
        m_MaxFramesInFlight = std::min(3U, static_cast<uint32_t>(m_VkSwapChainImageViews.size())); // This line added to grap image count
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_VkCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = m_MaxFramesInFlight; // Allocate one per frame
        m_VkCommandBuffers.resize(m_MaxFramesInFlight);
        if (vkAllocateCommandBuffers(m_VkDevice, &allocInfo, m_VkCommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }
    void Application::CreateSyncObjects() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // To stop the indefinite wait of the first fence
        m_VkImageAvailableSemaphores.resize(m_MaxFramesInFlight);
        m_VkRenderFinishedSemaphores.resize(m_MaxFramesInFlight);
        m_VkInFlightFences.resize(m_MaxFramesInFlight);
        for (size_t i = 0; i < m_MaxFramesInFlight; i++) {
            if (vkCreateSemaphore(m_VkDevice, &semaphoreInfo, nullptr, &m_VkImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_VkDevice, &semaphoreInfo, nullptr, &m_VkRenderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_VkDevice, &fenceInfo, nullptr, &m_VkInFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create semaphores and fences!");
            }
        }
    }

    void Application::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }
        {
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_VkRenderPass;
            renderPassInfo.framebuffer = m_VkSwapChainFramebuffers[imageIndex];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = m_VkSwapChainExtent;
    
            VkClearValue clearColor = {{{ 0.0f, 0.0f, 0.0f, 1.0f }}};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;
    
            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_VkGraphicsPipeline);
                VkViewport viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = static_cast<float>(m_VkSwapChainExtent.width);
                viewport.height = static_cast<float>(m_VkSwapChainExtent.height);
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        
                VkRect2D scissor{};
                scissor.offset = {0, 0};
                scissor.extent = m_VkSwapChainExtent;
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

                /* Draw */
                vkCmdDraw(commandBuffer, 3, 1, 0, 0);
            }
            vkCmdEndRenderPass(commandBuffer);
        }
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }
    void Application::DrawFrame() {
        vkWaitForFences(m_VkDevice, 1, &m_VkInFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_VkDevice, m_VkSwapchainKHR, UINT64_MAX, m_VkImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);
        
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized) {
            m_FramebufferResized = false;
            RecreateSwapChain();
            return; // Important so that invalid imageIndex in no further used
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image!");
        }
        
        vkResetFences(m_VkDevice, 1, &m_VkInFlightFences[m_CurrentFrame]);
        vkResetCommandBuffer(m_VkCommandBuffers[m_CurrentFrame], 0);
        RecordCommandBuffer(m_VkCommandBuffers[m_CurrentFrame], imageIndex);

        /* Submitting */
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkSemaphore waitSemaphores[] = { m_VkImageAvailableSemaphores[m_CurrentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_VkCommandBuffers[m_CurrentFrame];

        VkSemaphore signalSemaphores[] = { m_VkRenderFinishedSemaphores[m_CurrentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        if (vkQueueSubmit(m_VkGraphicsQueue, 1, &submitInfo, m_VkInFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { m_VkSwapchainKHR };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional
        
        result = vkQueuePresentKHR(m_VkPresentQueue, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            RecreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image!");
        }

        /* Rotation of frames */
        m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFramesInFlight;
    }

    /* For Swapchain recreation due to resizing or minimizing */
    void Application::CleanupSwapChain() {
        for(size_t i = 0; i < m_MaxFramesInFlight; i++) {
            vkDestroySemaphore(m_VkDevice, m_VkImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(m_VkDevice, m_VkRenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(m_VkDevice, m_VkInFlightFences[i], nullptr);
        }
        m_VkImageAvailableSemaphores.clear();
        m_VkRenderFinishedSemaphores.clear();
        m_VkInFlightFences.clear();
        for (auto framebuffer : m_VkSwapChainFramebuffers) {
            vkDestroyFramebuffer(m_VkDevice, framebuffer, nullptr);
        }
        m_VkSwapChainFramebuffers.clear();
        for (auto imageView : m_VkSwapChainImageViews) {
            vkDestroyImageView(m_VkDevice, imageView, nullptr);
        }
        m_VkSwapChainImageViews.clear();
        vkDestroySwapchainKHR(m_VkDevice, m_VkSwapchainKHR, nullptr);
    }
    void Application::RecreateSwapChain() {
        // Add window size event handler
        int width = 0, height = 0;
        glfwGetFramebufferSize(m_Window->GetNativeWindow(), &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_Window->GetNativeWindow(), &width, &height);
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(m_VkDevice);
        /* Cleanup */
        CleanupSwapChain(); // Cleanup + Command buffer manual freeing
        /* Recreation */
        CreateSwapChain();
        CreateImageViews();
        CreateFramebuffers();
        CreateSyncObjects();
    }

    bool Application::CheckInstanceExtensionSupport(const std::vector<const char*>& instanceExtensions) {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
        std::set<std::string> requiredExtensions(instanceExtensions.begin(), instanceExtensions.end());
        for (const auto& extension : availableExtensions) requiredExtensions.erase(extension.extensionName);
        return requiredExtensions.empty();
    }
    bool Application::CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto& extension : availableExtensions) requiredExtensions.erase(extension.extensionName);
        return requiredExtensions.empty();
    }
    bool Application::CheckInstanceLayerSupport(const std::vector<const char*>& instanceLayers) {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
        std::set<std::string> requiredLayers(instanceLayers.begin(), instanceLayers.end());
        for (const auto& layer : availableLayers) requiredLayers.erase(layer.layerName);
        return requiredLayers.empty();
    }
    bool Application::IsPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions) {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        return (
            // deviceFeatures.geometryShader &&
            (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) &&
            FindQueueFamilies(device, surface).IsComplete() &&
            CheckDeviceExtensionSupport(device, deviceExtensions) &&
            QuerySwapChainSupport(device, surface).IsAdequate()
        );
    }
    QueueFamilyIndices Application::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        int i = 0;
        for(const auto& queueFamily: queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.GraphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                indices.PresentFamily = i;
            }
            i++;
        }
        return indices;
    }
    SwapChainSupportDetails Application::QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.Formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.PresentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());
        }
        return details;
    }
    VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }
    VkPresentModeKHR Application::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { // Might want to add disabling this part when it comes to enegry efficiency --> Add to app config in future
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR; // vsync
    }
    VkExtent2D Application::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const std::unique_ptr<Window>& window) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window->GetNativeWindow(), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }
#ifdef INCLUDE_DEBUG_INFO
        VKAPI_ATTR VkBool32 VKAPI_CALL Application::DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        ) {

            LOG_ERROR("Validation layer: {}", pCallbackData->pMessage);

            return VK_FALSE;
        }
        VkResult Application::CreateDebugUtilsMessengerEXT(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger
        ) {
            auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
            if (func != nullptr) {
                return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
            } else {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        }
        void Application::DestroyDebugUtilsMessengerEXT(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator
        ) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr) {
                func(instance, debugMessenger, pAllocator);
            }
        }
#endif
}