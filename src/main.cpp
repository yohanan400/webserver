#include <thread>
#include <unistd.h>

#include "net/Acceptor.h"
#include "net/Socket.h"
#include "threadpool/ThreadPool.h"
#include "utils/Logger.h"

int main() {
    Logger::info("WebServer starting...");

    try
    {
        int cores = (int)sysconf(_SC_NPROCESSORS_ONLN);
        ThreadPool thread_pool(cores);
        Acceptor acceptor = Acceptor(5555);
        acceptor.bind();
        acceptor.listen();
        while (true)
        {
            Socket new_socket = acceptor.accept();
            auto shared_socket = std::make_shared<Socket>(std::move(new_socket));
            thread_pool.submit([shared_socket ]()
            {
                Logger::info("Socket accepted" + std::to_string(shared_socket->getFd()));
            });

        }
    }catch (std::exception& e)
    {
        Logger::error(e.what());
    }
    return 0;
}