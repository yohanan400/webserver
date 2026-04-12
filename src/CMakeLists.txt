add_library(webserver_lib
        net/Socket.cpp
        net/Acceptor.cpp
        #threadpool/ThreadPool.cpp
        utils/Logger.cpp
)

target_include_directories(webserver_lib
        PUBLIC ${CMAKE_SOURCE_DIR}/src
)

add_executable(webserver main.cpp)
target_link_libraries(webserver PRIVATE webserver_lib stdc++)