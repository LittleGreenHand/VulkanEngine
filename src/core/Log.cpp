#include "Log.h"

#define LOG_DEBUG(message) \
    VulkanLab::Log::Write( \
        VulkanLab::LogLevel::Debug, \
        message, \
        __FILE__, \
        __LINE__)

#define LOG_INFO(message) \
    VulkanLab::Log::Write( \
        VulkanLab::LogLevel::Info, \
        message, \
        __FILE__, \
        __LINE__)

#define LOG_WARNING(message) \
    VulkanLab::Log::Write( \
        VulkanLab::LogLevel::Warning, \
        message, \
        __FILE__, \
        __LINE__)

#define LOG_ERROR(message) \
    VulkanLab::Log::Write( \
        VulkanLab::LogLevel::Error, \
        message, \
        __FILE__, \
        __LINE__)

#define LOG_FATAL(message) \
    VulkanLab::Log::Write( \
        VulkanLab::LogLevel::Fatal, \
        message, \
        __FILE__, \
        __LINE__)

#ifndef NDEBUG

#define LOG_DEBUG(message) \
    VulkanLab::Log::Write( \
        VulkanLab::LogLevel::Debug, \
        message, \
        __FILE__, \
        __LINE__)

#else

#define LOG_DEBUG(message) ((void)0)

#endif