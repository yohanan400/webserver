#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "net/Socket.h"

class Router
{
public:
    Router() = default;
    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;

    void addRoute(const std::string& method, const std::string& path,
                  const std::function<void(const HttpRequest&, HttpResponse&,
                                           const std::shared_ptr<Socket>&)>& handler);
    void route(const HttpRequest& request, HttpResponse&, const std::shared_ptr<Socket>&);

private:
    std::unordered_map<std::string, std::function<void(const HttpRequest&, HttpResponse&,
                                                       const std::shared_ptr<Socket>&)>>
    _handlers_map;
};
