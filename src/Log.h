#pragma once
#include "pch.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace VulkanPractice {
    class Log {
    public:
        static void Init();
        inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }
    private:
        inline static std::shared_ptr<spdlog::logger> s_Logger;
    };
}

// core log macros
#define LOG_TRACE(...)      ::VulkanPractice::Log::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...)       ::VulkanPractice::Log::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)       ::VulkanPractice::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)      ::VulkanPractice::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...)   ::VulkanPractice::Log::GetLogger()->critical(__VA_ARGS__)

#define ENABLE_OSTREAM_FORMAT(Object)  template <> struct fmt::formatter<Object>: fmt::ostream_formatter {};  // Uses operator<<