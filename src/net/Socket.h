#pragma once
#include <string>

class Socket
{
public:
    Socket();
    Socket(const Socket&) = delete;
    Socket(Socket&&) noexcept;
    explicit Socket(int fd, std::string  ip) noexcept;
    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) noexcept;
    [[nodiscard]] int getFd() const;
    [[nodiscard]] std::string getIp() const;
    ~Socket();

private:
    int _fd;
    std::string _ip;
};
