#include <filesystem>
#include <fstream>
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
#include "utils/RateLimiter.h"

const std::unordered_map<std::string, std::string> mimeTypes = {
    {"html", "text/html; charset=utf-8"},
    {"htm", "text/html; charset=utf-8"},
    {"css", "text/css"},
    {"js", "text/javascript"},
    {"txt", "text/plain; charset=utf-8"},
    {"json", "application/json"},
    {"xml", "application/xml"},
    {"png", "image/png"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"gif", "image/gif"},
    {"svg", "image/svg+xml"},
    {"pdf", "application/pdf"},
    {"zip", "application/zip"},
    {"mp3", "audio/mpeg"},
    {"mp4", "video/mp4"}
};

int main()
{
    Logger::info("WebServer starting...");

    try
    {
        const int cores = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
        ThreadPool thread_pool(cores);

        auto router = Router();
        // Add Handler to "GET: /api/user"
        router.addRoute("GET", "/api/user",
                        [](const HttpRequest& request, HttpResponse& response, const std::shared_ptr<Socket>& socket)
                        {
                            response.setCode(200);
                            const std::string response_string = "{msg: this is the response}";
                            response.setBody(response_string);
                            response.setHeaders({
                                {"Content-Length", std::to_string(size(response_string))}
                            });
                        });

        // Add Handler to "GET: /static/"
        router.addRoute("GET", "/static/",
                        [](const HttpRequest& request, HttpResponse& response, const std::shared_ptr<Socket>& socket)
                        {
                            const std::string path = request.getPath();
                            std::string file_name = path.substr(path.find_last_of('/')+1);

                            const std::string file_path{"static/" + file_name};
                            const std::string file_type{file_name.substr(file_name.find('.')+1)};
                            const std::ifstream file{file_path, std::ios::in | std::ios::binary};

                            if (!file.is_open())
                            {
                                Logger::error("File " + file_name + " does not exise!");
                                response.setCode(404);
                                response.setBody("{msg: " + response.getCodeString());
                                return;
                            }

                            std::ostringstream ss;
                            ss << file.rdbuf();
                            response.setBody(ss.str());

                            if (mimeTypes.find(file_type) != mimeTypes.end())
                            {
                                std::string content_type {mimeTypes.find(file_type)->second};
                                response.setHeaders({
                                    {"Content-Type", content_type},
                                    {"Content-Length", std::to_string(std::size(response.getBody()))}
                                });
                                response.setCode(200);
                            }else
                            {
                                response.setHeaders({
                                    {"Content-Length", std::to_string(std::size(response.getBody()))}
                                });
                                response.setCode(415);
                            }
                        });

        auto acceptor = Acceptor(5555);
        acceptor.bind();
        acceptor.listen();

        auto mpl = MiddlewarePipeline();

        // Add Rate-Limiter middleware
        auto rate_limiter = RateLimiter(100, 60);
        mpl.use([&rate_limiter](const HttpRequest& http_request, HttpResponse& response,
                                const std::shared_ptr<Socket>& socket,
                                const Next& next)
        {
            if (rate_limiter.isAllowed(socket->getIp())) next();
            else
            {
                response.setCode(429);
                response.setBody("{msg: " + response.getCodeString() + "}");
                const std::string res = response.toString();
                ::send(socket->getFd(), res.c_str(), std::size(res), 0);
            }
        });

        // Add CORS middleware
        mpl.use([](const HttpRequest& http_request, HttpResponse& response, const std::shared_ptr<Socket>& socket,
                   const Next& next)
        {
            if (http_request.getMethod() == "OPTIONS")
            {
                response.setHeaders({
                    {"Access-Control-Allow-Origin", "*"},
                    {"Access-Control-Allow-Methods", "*"}
                });

                response.setCode(204);
                const std::string res = response.toString();
                ::send(socket->getFd(), res.c_str(), std::size(res), 0);
            }
            else
            {
                response.setHeaders({{"Access-Control-Allow-Origin", "*"}});
                next();
            }
        });

        // Add Logger middleware
        mpl.use([](const HttpRequest& http_request, HttpResponse& response, const std::shared_ptr<Socket>& socket,
                   const Next& next)
        {
            const auto start_time = std::chrono::steady_clock::now();
            next();
            const auto end_time = std::chrono::steady_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            Logger::info(
                http_request.getMethod() + " " + http_request.getPath() + " - " + std::to_string(duration.count()) +
                "ms");
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

                    HttpResponse response;
                    mpl.execute(request, response, shared_socket, [&router, request, &response, shared_socket]()
                    {
                        router.route(request, response, shared_socket);
                        const std::string res = response.toString();
                        ::send(shared_socket->getFd(), res.c_str(), res.length(), 0);
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
