#include "net/Acceptor.h"
#include "net/Socket.h"
#include "utils/Logger.h"

int main() {
    Logger::info("WebServer starting...");

    try
    {
        Acceptor acceptor = Acceptor(5555);
        acceptor.bind();
        acceptor.listen();
        while (true)
        {
            Socket new_socket = acceptor.accept();
            Logger::info("Socket accepted" + std::to_string(new_socket.getFd()));
        }
    }catch (std::exception& e)
    {
        Logger::error(e.what());
    }
    return 0;
}