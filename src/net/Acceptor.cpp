#include "Acceptor.h"
#include <cstring>
#include <stdexcept>
#include <netinet/in.h>
#include <cerrno>
#include "Socket.h"
#include "utils/Logger.h"

Acceptor::Acceptor(const int port, const int backlog) : _port(port), _backlog(backlog)
{
    _socket = Socket();
    constexpr int optval = 1;
    int result = setsockopt(_socket.getFd(),SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (result == -1)
    {
        Logger::error("setsockopt SO_REUSEADDR failed");
        throw std::runtime_error("setsockopt SO_REUSEADDR failed");
    }
}

void Acceptor::bind()
{
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(_port);

    int result = ::bind(_socket.getFd(), reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));

    if (result == -1)
    {
        Logger::error("bind() failed. Error number: " + std::to_string(errno));
        throw std::runtime_error("bind() failed");
    }
}

void Acceptor::listen()
{
    int result = ::listen(_socket.getFd(), _backlog);

    if (result == -1)
    {
        Logger::error("listen() failed. Error number: " + std::to_string(errno));
        throw std::runtime_error("listen() failed");
    }
}

Socket Acceptor::accept()
{
    sockaddr_storage their_addr{};
    socklen_t their_addr_len = sizeof(their_addr);
    int result = ::accept(_socket.getFd(), reinterpret_cast<struct sockaddr*>(&their_addr), &their_addr_len);

    if (result == -1)
    {
        Logger::error("accept() failed. Error number: " + std::to_string(errno));
        throw std::runtime_error("accept() failed");
    }

    return Socket(result);
}
