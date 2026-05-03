#include "RateLimiter.h"

#include <stdexcept>

RateLimiter::RateLimiter(const int max_rate, const int max_time_interval)
{
    if (max_rate >= 0) _max_rate = max_rate;
    else throw std::runtime_error("Max rate number < 0!");

    if (max_time_interval >= 0) _max_time_interval = max_time_interval;
    else throw std::runtime_error("Max time interval number < 0!");
}

bool RateLimiter::isInTimeInterval(const std::chrono::steady_clock::time_point& stored) const
{
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - stored).count() <= _max_time_interval;
}

bool RateLimiter::isAllowed(const std::string& ip)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (const auto result = _intervals.find(ip); (result != _intervals.end()))
    {
        if (isInTimeInterval(result->second.second))
        {
            if (result->second.first >= _max_rate) return false;

            result->second.first += 1;
            return true;
        }
        result->second.first = 1;
        result->second.second = std::chrono::steady_clock::now();
        return true;
    }


    _intervals.insert({ip, {1, std::chrono::steady_clock::now()}});
    return true;
}
