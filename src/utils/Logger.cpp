#include "Logger.h"
#include <iostream>
#include <chrono>
#include <ctime>

static std::string timestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));
    return buf;
}

void Logger::info(const std::string& msg)
{
    std::cout << "[" << timestamp() << "] [INFO]  " << msg << "\n";
}

void Logger::error(const std::string& msg)
{
    std::cerr << "[" << timestamp() << "] [ERROR] " << msg << "\n";
}

void Logger::debug(const std::string& msg)
{
    std::cout << "[" << timestamp() << "] [DEBUG] " << msg << "\n";
}
