#pragma once
#include <functional>
#include <memory>

#include "http/HttpRequest.h"
#include "net/Socket.h"

using Next = std::function<void()>;
using Middleware = std::function<void(const HttpRequest&, const std::shared_ptr<Socket>&, Next)>;

class MiddlewarePipeline
{
public:
    void use(const Middleware&);
    void execute(const HttpRequest& request, const std::shared_ptr<Socket>& socket, const Next& final_handler) const;

private:
    std::vector<Middleware> _middlewares;
};
