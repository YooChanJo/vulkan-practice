#include "Window.h"

namespace VulkanPractice {
    Window::Window(uint32_t width, uint32_t height, const std::string& title)
        : m_Width(width), m_Height(height), m_Title(title)
    {
        /* Check if an instance already exists */
        if(s_Instance != nullptr) {
            throw std::runtime_error("A Window already exists");
        }
        /* Initialize GLFW */
        if(!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        /* Hint that we are not using openGL */
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // window resize disabled for now

        m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
        if(!m_Window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create window");
        }
        s_Instance = this;
    }
    Window::~Window() {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
        s_Instance = nullptr;
    }
}