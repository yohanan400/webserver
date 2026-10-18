#include "EventLoop.h"
#include <sys/epoll.h>
#include <utils/Logger.h>
#include <fcntl.h>
#include "http/HttpParser.h"

EventLoop::EventLoop(MiddlewarePipeline& middleware, ThreadPool& thread_pool, Acceptor& acceptor):
                _middleware(middleware), _threadPool(thread_pool), _acceptor(acceptor)
{
    _epollFd = epoll_create1(0);
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = _acceptor.getFd();
    epoll_ctl(_epollFd, EPOLL_CTL_ADD, _acceptor.getFd(), &ev);
}

void EventLoop::addConnection(std::shared_ptr<SSLSocket> conn)
{
    std::lock_guard<std::mutex> lock(_mutex);
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = conn->getFd();
    int flags = fcntl(conn->getFd(), F_GETFL, 0);
    fcntl(conn->getFd(), F_SETFL, flags | O_NONBLOCK);

    epoll_ctl(_epollFd, EPOLL_CTL_ADD, conn->getFd(), &ev);
    _connections[conn->getFd()] = conn;
}

void EventLoop::removeConnection(std::shared_ptr<SSLSocket> conn)
{
    std::lock_guard<std::mutex> lock(_mutex);
    epoll_ctl(_epollFd, EPOLL_CTL_DEL, conn->getFd(), nullptr);
    _connections.erase(conn->getFd());
}

void EventLoop::loop(Router& router)
{
    constexpr  int MAX_EVENTS = 1024;
    epoll_event events[MAX_EVENTS];
    while (true)
    {
        int connections = epoll_wait(_epollFd, events, MAX_EVENTS, 5000);
        if (connections == -1) Logger::error("epoll_wait failed");
        else for (int i = 0; i < connections; i++)
        {
            if (events[i].data.fd == _acceptor.getFd())
            {
                try
                {
                    SSLSocket new_socket = _acceptor.accept();
                    auto shared_socket = std::make_shared<SSLSocket>(SSLSocket(std::move(new_socket)));

                    addConnection(shared_socket);
                }
                catch (std::exception& e)
                {
                    Logger::error(e.what());
                }
            }
            else {
                auto shared_socket = _connections[events[i].data.fd];

                _threadPool.submit([shared_socket, &router, this]()
                {
                    Logger::info("Socket accepted " + std::to_string(shared_socket->getFd()));
                    constexpr size_t MAX_BUFFER_SIZE = 4096;

                    while (true)
                    {
                        std::string buffer(MAX_BUFFER_SIZE, '\0');
                        ssize_t result = shared_socket->recv(buffer.data(), buffer.size(), 0);

                        if (result == 0)
                        {
                            Logger::info("Connection closed by client");
                            removeConnection(shared_socket);
                            break;
                        }

                        if (result == -1)
                        {
                            if (errno == EAGAIN) Logger::info("Done reading");
                            else
                            {
                                Logger::error("Error occur while tring to receive data!");
                                removeConnection(shared_socket);
                            }
                            break;
                        }
                        buffer.resize(result);

                        Logger::info("recv success!");
                        Logger::info(buffer);

                        HttpParser parser;
                        HttpRequest request = parser.parse(buffer);

                        HttpResponse response;
                        _middleware.execute(request, response, shared_socket,
                                    [&router, request, &response, shared_socket]()
                                    {
                                        router.route(request, response, shared_socket);
                                        const std::string res = response.toString();
                                        shared_socket->send( res.c_str(), res.length(), 0);
                                    });
                    }
                });
            }
        }
    }
}