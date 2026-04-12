#pragma once

class Socket
{
    public:
        Socket();
        Socket(const Socket&) = delete;
        Socket(Socket&&) noexcept;
        explicit Socket(int fd) noexcept;
        Socket& operator=(const Socket&) = delete;
        Socket& operator=(Socket&&) noexcept;
        [[nodiscard]] int getFd() const;
        ~Socket();

    private:
        int _fd;
};