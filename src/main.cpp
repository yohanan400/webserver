#include <thread>
#include <unistd.h>
#include <sys/socket.h>

#include "http/HttpParser.h"
#include "http/HttpRequest.h"
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
            thread_pool.submit([shared_socket]()
            {
                Logger::info("Socket accepted " + std::to_string(shared_socket->getFd()));

                std::string buffer(4096, '\0');
                ssize_t result = ::recv(shared_socket->getFd(), buffer.data(), buffer.size(), 0);
                buffer.resize(result);

                if (result <= 0) Logger::info("recv failed!");
                else
                {
                    Logger::info("recv success!");

                    Logger::info(buffer);
                    HttpParser parser;
                    HttpRequest request = parser.parse(buffer);

                    Logger::info("method: " +request.get_method());
                    Logger::info("path: " +request.get_path());
                }



            });

        }
    }catch (std::exception& e)
    {
        Logger::error(e.what());
    }
    return 0;
}