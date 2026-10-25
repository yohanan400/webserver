#include "Logger.h"
#include <iostream>
#include <chrono>
#include <ctime>

LogLevel Logger::_logLevel = LogLevel::DEBUG;

static std::string timestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));
    return buf;
}

void Logger::debug(const std::string& msg)
{
    if (_logLevel <= LogLevel::DEBUG)
    std::cout << "[" << timestamp() << "] [DEBUG] " << msg << "\n";
}

void Logger::info(const std::string& msg)
{
    if (_logLevel <= LogLevel::INFO)
    std::cout << "[" << timestamp() << "] [INFO]  " << msg << "\n";
}

void Logger::error(const std::string& msg)
{
    if (_logLevel <= LogLevel::ERROR)
    std::cerr << "[" << timestamp() << "] [ERROR] " << msg << "\n";
}

void Logger::setLogLevel(const LogLevel level)
{
    _logLevel = level;
}