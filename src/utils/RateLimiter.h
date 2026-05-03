#pragma once
#include <mutex>
#include <string>
#include <unordered_map>

class RateLimiter
{
public:
    RateLimiter(int max_rate, int max_time_interval);
    bool isAllowed(const std::string& ip);

private:
    int _max_rate;
    int _max_time_interval; // In seconds
    std::mutex _mutex;
    std::unordered_map<std::string, std::pair<int, std::chrono::steady_clock::time_point>> _intervals;

    bool isInTimeInterval(const std::chrono::steady_clock::time_point& stored) const;
};
