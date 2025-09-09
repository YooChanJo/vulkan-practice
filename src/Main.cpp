#include "pch.h"
#include "Application.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

using namespace VulkanPractice;
int main() {
    Application MyApp;
    try {
        MyApp.Run();
    } catch (const std::exception& e) {
        LOG_CRITICAL(e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}