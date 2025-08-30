#pragma once
/* This Header handles GLFW; window management */
#include "pch.h"
#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

namespace VulkanPractice {
    class Window {
    private:
        inline static Window* s_Instance = nullptr;

        GLFWwindow* m_Window;
        uint32_t m_Width, m_Height;
        std::string m_Title;
    public:
        Window(uint32_t width, uint32_t height, const std::string& title);
        ~Window();

        inline static Window* GetInstance() { return s_Instance; }
        inline GLFWwindow* GetNativeWindow() const { return m_Window; }
        inline uint32_t GetWidth() const { return m_Width; }
        inline uint32_t GetHeight() const { return m_Height; }
        inline const std::string& GetTitle() const { return m_Title; }
    };
}