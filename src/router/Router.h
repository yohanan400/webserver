#pragma once
#include <functional>
#include <string>
#include <unordered_map>

#include "http/HttpRequest.h"

class Router
{
public:
    Router() = default;
    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;

    void addRoute(const std::string& method, const std::string& path, const std::function<void(const HttpRequest&)>& handler);
    void route(const HttpRequest& request);

private:
    std::unordered_map<std::string, std::function<void(const HttpRequest&)>> _handlers_map;
};
