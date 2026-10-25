#pragma once
#include <string>

enum class LogLevel {DEBUG, INFO, ERROR};

class Logger
{
public:
    static void info(const std::string& msg);
    static void error(const std::string& msg);
    static void debug(const std::string& msg);
    static void setLogLevel(LogLevel level);

private:
    static LogLevel _logLevel;
};
