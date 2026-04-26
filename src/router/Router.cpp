#include "Router.h"

#include "utils/Logger.h"

void Router::addRoute(const std::string& method, const std::string& path, const std::function<void(const HttpRequest&)>& handler)
{
    _handlers_map[method + ":" + path] = handler;
}

void Router::route(const HttpRequest& request)
{
    std::string full_path = request.getMethod() + ":" + request.getPath();

    const auto it = _handlers_map.find(full_path);
    if (it == _handlers_map.end())
    {
        Logger::error("Route " + full_path + " does not exist");
        return;
    }
    it->second(request);
}