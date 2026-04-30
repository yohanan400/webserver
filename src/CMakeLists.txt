add_library(webserver_lib
        net/Socket.cpp
        net/Acceptor.cpp
        #threadpool/ThreadPool.cpp
        utils/Logger.cpp
)

target_include_directories(webserver_lib
        PUBLIC ${CMAKE_SOURCE_DIR}/src
)

add_executable(webserver main.cpp
        threadpool/ThreadPool.cpp
        threadpool/ThreadPool.h
        http/HttpRequest.cpp
        http/HttpRequest.h
        http/HttpParser.cpp
        http/HttpParser.h
        router/Router.cpp
        router/Router.h
        http/HttpResponse.cpp
        http/HttpResponse.h
        middleware/Middleware.cpp
        middleware/Middleware.h)
target_link_libraries(webserver PRIVATE webserver_lib stdc++)