#pragma once
#include "pch.h"
#include "Window.h"
#include "Log.h"

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
#ifdef INCLUDE_DEBUG_INFO
        VkDebugUtilsMessengerEXT m_VkDebugUtilsMessengerEXT;
#endif

        VkPhysicalDevice m_VkPhysicalDevice;

        VkDevice m_VkDevice;
        VkQueue m_VkGraphicsQueue, m_VkPresentQueue;

        VkSwapchainKHR m_VkSwapchainKHR;
        std::vector<VkImage> m_SwapChainImages;
        VkFormat m_VkSwapChainImageFormat;
        VkExtent2D m_VkSwapChainExtent;
        std::vector<VkImageView> m_VkSwapChainImageViews;
        VkRenderPass m_VkRenderPass;
        VkPipelineLayout m_VkPipelineLayout;
        VkPipeline m_VkGraphicsPipeline;
        std::vector<VkFramebuffer> m_VkSwapChainFramebuffers;

        uint32_t m_MaxFramesInFlight; // consider changing this to 3 --> and this does not consider GPU needs pathc
        VkCommandPool m_VkCommandPool;
        std::vector<VkCommandBuffer> m_VkCommandBuffers;

        std::vector<VkSemaphore> m_VkImageAvailableSemaphores;
        std::vector<VkSemaphore> m_VkRenderFinishedSemaphores;
        std::vector<VkFence> m_VkInFlightFences;
        size_t m_CurrentFrame = 0; // for tracking

        bool m_FramebufferResized = false;

        // possible add this to app config
        std::vector<const char*> m_InstanceExtensions;
        std::vector<const char*> m_DeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME, // required
        };
        std::vector<const char*> m_InstanceLayers = {
#ifdef INCLUDE_DEBUG_INFO
            "VK_LAYER_KHRONOS_validation",
#endif
        };
        // std::vector<const char*> m_DeviceLayers;
    public:
        Application(const ApplicationConfig& config = ApplicationConfig());
        ~Application();

        void Run();

        inline static Application* GetInstance() { return s_Instance; }
        inline const std::unique_ptr<Window>& GetWindow() const { return m_Window; }
        inline const std::string& GetApplicationName() const { return m_ApplicationName; }
        inline const std::string& GetApplicationEngineName() const { return m_ApplicationEngineName; }
    private:
        void InitVulkan();
        void CleanupVulkan();

        void CreateInstance();
#ifdef INCLUDE_DEBUG_INFO
        void SetupDebugMessenger();
#endif
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateSwapChain();
        void CreateImageViews();
        void CreateRenderPass();
        void CreateGraphicsPipeline();
        void CreateFramebuffers();
        void CreateCommandPool();
        void CreateCommandBuffers();
        void CreateSyncObjects();

        void CleanupSwapChain(); // Handles window size changes etc
        void RecreateSwapChain(); // Handles window size changes etc

        void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        void DrawFrame();

        /* Util functions */
        static bool CheckInstanceExtensionSupport(const std::vector<const char*>& instanceExtensions);
        static bool CheckDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions);
        static bool CheckInstanceLayerSupport(const std::vector<const char*>& instanceLayers);

        static bool IsPhysicalDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
        static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
        static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const std::unique_ptr<Window>& window);

        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height); // glfw frame buffer callback function
#ifdef INCLUDE_DEBUG_INFO
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        );
        static VkResult CreateDebugUtilsMessengerEXT(
            VkInstance instance,
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator,
            VkDebugUtilsMessengerEXT* pDebugMessenger
        );
        static void DestroyDebugUtilsMessengerEXT(
            VkInstance instance,
            VkDebugUtilsMessengerEXT debugMessenger,
            const VkAllocationCallbacks* pAllocator
        );
#endif
    };
}