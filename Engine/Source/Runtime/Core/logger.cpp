// Copied from Cherno's [OpenGL-Core template](https://github.com/TheCherno/OpenGL)
#include "logger.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace gdp1 {

std::shared_ptr<spdlog::logger> Logger::s_Logger;

void Logger::Init() {
    spdlog::set_pattern("%^[%T] %n: %v%$");
    s_Logger = spdlog::stdout_color_mt("MiniEngine");
    s_Logger->set_level(spdlog::level::trace);
}

}  // namespace gdp1
