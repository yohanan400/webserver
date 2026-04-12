#pragma once
#include "Socket.h"

class Acceptor
{
    public:
        Acceptor(int port, int backlog = 20);
        ~Acceptor() = default;
        void bind();
        void listen();
        Socket accept();

    private:
        Socket _socket;
        int _port;
        int _backlog;
};
