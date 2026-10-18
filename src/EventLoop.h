#pragma once
#include <memory>

#include "middleware/Middleware.h"
#include "net/Acceptor.h"
#include "threadpool/ThreadPool.h"
#include "router/Router.h"
class EventLoop
{
public:
    EventLoop(MiddlewarePipeline&, ThreadPool&, Acceptor&);
    void removeConnection(std::shared_ptr<SSLSocket> conn);
    void loop(Router&);

private:
    MiddlewarePipeline& _middleware;
    ThreadPool& _threadPool;
    Acceptor& _acceptor;
    int _epollFd;
    std::mutex _mutex;
    std::unordered_map<int, std::shared_ptr<SSLSocket>> _connections;
    void addConnection(std::shared_ptr<SSLSocket> conn);
};
