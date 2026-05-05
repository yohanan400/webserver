#include "Router.h"

#include "utils/Logger.h"

void Router::addRoute(const std::string& method, const std::string& path,
                      const std::function<void(const HttpRequest&, HttpResponse& response,
                                               const std::shared_ptr<Socket>&)>& handler)
{
    _handlers_map[method + ":" + path] = handler;
}

void Router::route(const HttpRequest& request, HttpResponse& response, const std::shared_ptr<Socket>& socket)
{
    const std::string full_path = request.getMethod() + ":" + request.getPath();

    auto it = _handlers_map.find(full_path);
    if (it == _handlers_map.end())
    {
        if (request.getPath().find("/static/") == 0)
        {
            it = _handlers_map.find("GET:/static/");

            if (it == _handlers_map.end())
            {
                Logger::error("Route " + full_path + " does not exist");
                return;
            }

            it->second(request, response, socket);
            return;
        }
        Logger::error("Route " + full_path + " does not exist");
        return;
    }
    it->second(request, response, socket);
}
