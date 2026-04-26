#include <thread>
#include <unistd.h>
#include <sys/socket.h>

#include "http/HttpParser.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "net/Acceptor.h"
#include "net/Socket.h"
#include "threadpool/ThreadPool.h"
#include "utils/Logger.h"
#include "router/Router.h"


int main() {
    Logger::info("WebServer starting...");

    try
    {
        int cores = (int)sysconf(_SC_NPROCESSORS_ONLN);
        ThreadPool thread_pool(cores);

        Router router = Router();
        router.addRoute("GET", "/api/user", \
            [](const HttpRequest& request, const std::shared_ptr<Socket>& socket){\
                Logger::info("method: " +request.getMethod());\
                Logger::info("path: " +request.getPath());\

                 HttpResponse response;
                response.setCode(200);
                const std::string response_string = "{msg: this is the response}";
                response.setBody(response_string);
                response.setHeaders({{"Content-Type", "application/json"}, \
                    {"Content-Length", std::to_string(size(response_string))}});
                const std::string res = response.toString();
                ::send(socket->getFd(), res.c_str(), res.length(), 0);
        });

        Acceptor acceptor = Acceptor(5555);
        acceptor.bind();
        acceptor.listen();
        while (true)
        {
            Socket new_socket = acceptor.accept();
            auto shared_socket = std::make_shared<Socket>(std::move(new_socket));
            thread_pool.submit([shared_socket, &router]()
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

                    router.route(request, shared_socket);
                }
            });

        }
    }catch (std::exception& e)
    {
        Logger::error(e.what());
    }
    return 0;
}