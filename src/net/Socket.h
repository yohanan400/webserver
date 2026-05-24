#pragma once
#include <string>

class Socket
{
public:
    Socket();
    Socket(const Socket&) = delete;
    Socket(Socket&&) noexcept;
    explicit Socket(int fd, std::string ip) noexcept;
    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) noexcept;
    virtual ssize_t send(const void* buf,size_t n,int flags);
    virtual ssize_t recv(void* buf, size_t n, int flags);
    [[nodiscard]]
    int getFd() const;
    [[nodiscard]] std::string getIp() const;
    void setTimeOut(int tv_sec) const;
    virtual ~Socket();

protected:
    int _fd;
    std::string _ip;
};
