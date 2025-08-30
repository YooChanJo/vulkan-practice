#pragma once
#include "pch.h"
#include "Window.h"

namespace VulkanPractice {
    struct ApplicationConfig {
        std::string ApplicationName = "Vulkan Application";
        std::string ApplicationEngineName = "No Engine";

        uint32_t WindowWidth = 1280;
        uint32_t WindowHeight = 720;
        std::string WindowTitle = "Vulkan";
    };

    struct QueueFamilyIndices {
        std::optional<uint32_t> GraphicsFamily; // use .has_value() to get true/false --> can see whether a value is assigned
        std::optional<uint32_t> PresentFamily;

        inline bool IsComplete() const { return GraphicsFamily.has_value() && PresentFamily.has_value(); }
    };
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR Capabilities;
        std::vector<VkSurfaceFormatKHR> Formats;
        std::vector<VkPresentModeKHR> PresentModes;

        inline bool IsAdequate() const { return !Formats.empty() && !PresentModes.empty(); }
    };


    class Application {
    private:
        inline static Application* s_Instance = nullptr;

        std::string m_ApplicationName, m_ApplicationEngineName;
        std::unique_ptr<Window> m_Window;

        VkInstance m_VkInstance;
        VkSurfaceKHR m_VkSurfaceKHR;
        VkPhysicalDevice m_VkPhysicalDevice;
        VkDevice m_VkDevice;
        VkSwapchainKHR m_VkSwapchainKHR;
        VkQueue m_GraphicsQueue, m_PresentQueue;

        std::vector<const char*> m_DeviceExtensions = { // possible add this to app config
            VK_KHR_SWAPCHAIN_EXTENSION_NAME, // currently default
        };
    public:
        Application(const ApplicationConfig& config = ApplicationConfig());
        ~Application();

        inline static Application* GetInstance() { return s_Instance; }
        inline const std::unique_ptr<Window>& GetWindow() const { return m_Window; }
        inline const std::string& GetApplicationName() const { return m_ApplicationName; }
        inline const std::string& GetApplicationEngineName() const { return m_ApplicationEngineName; }
    private:
        void InitVulkan();
        void CleanupVulkan();

        /* Util functions */
        static bool IsPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
        static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);
        static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
        static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const std::unique_ptr<Window>& window);
    };
}