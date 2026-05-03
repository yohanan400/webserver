#include "Socket.h"
#include <unistd.h>
#include <sys/socket.h>
#include "utils/Logger.h"
#include <stdexcept>
#include <utility>

Socket::Socket()
{
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1)
    {
        Logger::error("File descriptor has failed to create");
        throw std::runtime_error("Failed to create socket");
    }
}

Socket::Socket(Socket&& other) noexcept : _fd(other._fd)
{
    other._fd = -1;
}

Socket::Socket(const int fd, std::string ip) noexcept : _fd(fd), _ip(std::move(ip))
{
}

Socket& Socket::operator=(Socket&& other) noexcept
{
    if (this != &other)
    {
        close(_fd);
        _fd = other._fd;
        other._fd = -1;
    }

    return *this;
}

int Socket::getFd() const
{
    return _fd;
}

std::string Socket::getIp() const
{
    return _ip;
}

Socket::~Socket()
{
    if (_fd != -1)
    {
        close(_fd);
    }
}
