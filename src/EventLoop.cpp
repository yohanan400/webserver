#include "EventLoop.h"
#include <sys/epoll.h>
#include <utils/Logger.h>
#include <fcntl.h>
#include <stdexcept>

#include "http/HttpParser.h"

EventLoop::EventLoop(MiddlewarePipeline& middleware, ThreadPool& thread_pool, Acceptor& acceptor):
                _middleware(middleware), _threadPool(thread_pool), _acceptor(acceptor)
{
    _epollFd = epoll_create1(0);
    if (_epollFd == -1)
        throw std::runtime_error("epoll_create1 failed: " + std::string(strerror(errno)));

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = _acceptor.getFd();
    auto result = epoll_ctl(_epollFd, EPOLL_CTL_ADD, _acceptor.getFd(), &ev);

    if (result == -1)
        throw std::runtime_error("epoll_ctl failed: " + std::string(strerror(errno)));
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
                std::shared_ptr<SSLSocket> shared_socket;
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    auto it = _connections.find(events[i].data.fd);
                    if (it == _connections.end()) continue;
                    shared_socket = it->second;
                }

                if (events[i].events & (EPOLLHUP | EPOLLERR)) {
                    removeConnection(shared_socket);
                    continue;
                }

                _threadPool.submit([shared_socket, &router, this]()
                {
                    Logger::info("Socket accepted " + std::to_string(shared_socket->getFd()));
                    constexpr size_t MAX_BUFFER_SIZE = 4096;

                    try
                    {
                        while (true)
                        {
                            std::string buffer(MAX_BUFFER_SIZE, '\0');
                            ssize_t result;
                            while (true)
                            {
                                result = shared_socket->recv(buffer.data(), MAX_BUFFER_SIZE, 0);
                                if (result > 0) shared_socket->_buffer += buffer.substr(0, result);
                                else break;
                            }

                            if (result == 0)
                            {
                                Logger::info("Connection closed by client");
                                removeConnection(shared_socket);
                                break;
                            }

                            if (result == -1)
                            {
                                if (errno == EAGAIN)
                                {
                                    if (shared_socket->_buffer.find("\r\n\r\n") == std::string::npos)
                                    {
                                        Logger::info("EAGAIN received before reading all. Pause socket reading.");
                                        continue;
                                    }
                                }
                                else
                                {
                                    Logger::error("Error occur while tring to receive data!");
                                    removeConnection(shared_socket);
                                    break;
                                }
                            }

                            Logger::info("recv success!");
                            Logger::info(shared_socket->_buffer);

                            HttpParser parser;
                            HttpRequest request = parser.parse(shared_socket->_buffer);

                            shared_socket->_buffer.clear();

                        HttpResponse response;
                        _middleware.execute(request, response, shared_socket,
                                    [&router, request, &response, shared_socket]()
                                    {
                                        router.route(request, response, shared_socket);
                                        const std::string res = response.toString();
                                        shared_socket->send( res.c_str(), res.length(), 0);
                                    });
                        }
                    }
                    catch (std::exception& e)
                    {
                        Logger::error("Connection Error: " + std::string(e.what()));
                        removeConnection(shared_socket);
                    }
                });
            }
        }
    }
}