#include "Application.h"

namespace VulkanPractice {
    Application::Application(const ApplicationConfig& config)
        : m_ApplicationName(config.ApplicationName), m_ApplicationEngineName(config.ApplicationEngineName)
    {
        if(s_Instance != nullptr) {
            throw std::runtime_error("An Application already exists");
        }
        m_Window = std::make_unique<Window>(config.WindowWidth, config.WindowHeight, config.WindowTitle);
        Log::Init();
        InitVulkan();
        /* Print Extensions Info */
        DEBUG_ONLY(
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
        )
        s_Instance = this;
    }
    Application::~Application() {
        CleanupVulkan();
        s_Instance = nullptr;
    }

    void Application::InitVulkan() {
        /* Create Vulkan Instance */
        CreateInstance();
        /* Setup Debug Messenger only for Debug */
        DEBUG_ONLY(SetupDebugMessenger();)
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
        /* Create Graphics Pipeline */
        CreateGraphicsPipeline();
    }
    void Application::CleanupVulkan() {
        for (auto imageView : m_SwapChainImageViews) {
            vkDestroyImageView(m_VkDevice, imageView, nullptr);
        }
        vkDestroySwapchainKHR(m_VkDevice, m_VkSwapchainKHR, nullptr);
        vkDestroyDevice(m_VkDevice, nullptr);
        vkDestroySurfaceKHR(m_VkInstance, m_VkSurfaceKHR, nullptr);
        DEBUG_ONLY(DestroyDebugUtilsMessengerEXT(m_VkInstance, m_VkDebugUtilsMessengerEXT, nullptr);)
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
            DEBUG_ONLY(m_InstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME));
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
        DEBUG_ONLY(
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = DebugCallback;
            debugCreateInfo.pUserData = nullptr; // Optional
            debugCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        )

        if (vkCreateInstance(&createInfo, nullptr, &m_VkInstance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vkinstance!");
        }
    }
    DEBUG_ONLY(
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
    )
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
        vkGetDeviceQueue(m_VkDevice, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_VkDevice, indices.PresentFamily.value(), 0, &m_PresentQueue);

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
        m_SwapChainImageFormat = surfaceFormat.format;
        m_SwapChainExtent = extent;
    }
    void Application::CreateImageViews() {
        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_SwapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_SwapChainImageFormat;
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
            if (vkCreateImageView(m_VkDevice, &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image views!");
            }
        }
    }
    void Application::CreateGraphicsPipeline() {
        
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
    DEBUG_ONLY(
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
    )
}