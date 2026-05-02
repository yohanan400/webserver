#include <thread>
#include <unistd.h>
#include <sys/socket.h>

#include "http/HttpParser.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "middleware/Middleware.h"
#include "net/Acceptor.h"
#include "net/Socket.h"
#include "threadpool/ThreadPool.h"
#include "utils/Logger.h"
#include "router/Router.h"


int main()
{
    Logger::info("WebServer starting...");

    try
    {
        const int cores = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
        ThreadPool thread_pool(cores);

        auto router = Router();
        router.addRoute("GET", "/api/user",
                        [](const HttpRequest& request, const std::shared_ptr<Socket>& socket)
                        {
                            HttpResponse response;
                            response.setCode(200);
                            const std::string response_string = "{msg: this is the response}";
                            response.setBody(response_string);
                            response.setHeaders({
                                {"Content-Type", "application/json"},
                                {"Content-Length", std::to_string(size(response_string))}
                            });
                            const std::string res = response.toString();
                            ::send(socket->getFd(), res.c_str(), res.length(), 0);
                        });

        auto acceptor = Acceptor(5555);
        acceptor.bind();
        acceptor.listen();

        auto mpl = MiddlewarePipeline();
        // Add one middleware for example
        mpl.use([](const HttpRequest& http_request, const std::shared_ptr<Socket>& socket, const Next& next)
        {
            Logger::info("method: " + http_request.getMethod());\
            Logger::info("path: " + http_request.getPath());

            next();
        });

        while (true)
        {
            Socket new_socket = acceptor.accept();
            auto shared_socket = std::make_shared<Socket>(std::move(new_socket));
            thread_pool.submit([&mpl, shared_socket, &router]()
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

                    mpl.execute(request, shared_socket, [&router, request, shared_socket]()
                    {
                        router.route(request, shared_socket);
                    });
                }
            });
        }
    }
    catch (std::exception& e)
    {
        Logger::error(e.what());
    }

    return 0;
}
