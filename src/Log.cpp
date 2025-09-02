#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace VulkanPractice {
    void Log::Init() {
        const auto logger = spdlog::get("Vulkan Practice");
        if(logger) return;
        spdlog::set_pattern("[%T] [%^%l%$] %v");
        s_Logger = spdlog::stdout_color_mt("Vulkan Practice");
        s_Logger->set_level(spdlog::level::trace);
    }
}