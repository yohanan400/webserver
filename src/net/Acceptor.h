#pragma once
#include "Socket.h"
#include "SSLSocket.h"

class Acceptor
{
public:
    explicit Acceptor(int port, int backlog = 20);
    ~Acceptor() = default;
    void bind() const;
    void listen() const;
    [[nodiscard]] SSLSocket accept() const;

private:
    Socket _socket;
    int _port;
    int _backlog;
};
