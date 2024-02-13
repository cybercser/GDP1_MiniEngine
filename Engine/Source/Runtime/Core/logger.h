#pragma once
// Copied from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL)

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace gdp1 {

class Logger {
public:
    static void Init();

    inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

private:
    static std::shared_ptr<spdlog::logger> s_Logger;
};

}  // namespace gdp1

// log macros
#define LOG_TRACE(...) ::gdp1::Logger::GetLogger()->trace(__VA_ARGS__)
#define LOG_INFO(...) ::gdp1::Logger::GetLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) ::gdp1::Logger::GetLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...) ::gdp1::Logger::GetLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::gdp1::Logger::GetLogger()->critical(__VA_ARGS__)
