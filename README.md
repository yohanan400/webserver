[To be honest.. I was to lazy to write down all this// so this is claude made ;)]

# HTTP Web Server in C++17

A fully functional HTTP/1.1 web server written from scratch in C++17, without any external server libraries. Built as a portfolio project demonstrating advanced systems programming concepts including multithreading, network programming, and design patterns.

---

## Features

- **TCP Server** — raw socket programming with `epoll`-ready architecture
- **Thread Pool** — concurrent request handling with work queue and graceful shutdown
- **HTTP Parser** — state-based parsing of HTTP/1.1 requests including headers, query strings, and body
- **Router** — fast hash-map based routing with support for multiple HTTP methods
- **HTTP Response** — structured response builder with automatic status text resolution
- **RAII throughout** — all resources (sockets, threads) managed automatically via C++ destructors

---

## Architecture

```
Client Request
      ↓
  Acceptor Thread        (net/Acceptor)
      ↓
  Thread Pool            (threadpool/ThreadPool)
      ↓
  HTTP Parser            (http/HttpParser)
      ↓
  Router                 (router/Router)
      ↓
  Handler → HttpResponse (http/HttpResponse)
      ↓
  send() → Client
```

---

## Project Structure

```
webserver/
├── src/
│   ├── net/
│   │   ├── Socket.hpp / Socket.cpp       # RAII TCP socket wrapper
│   │   └── Acceptor.hpp / Acceptor.cpp   # bind, listen, accept
│   ├── http/
│   │   ├── HttpRequest.hpp / .cpp        # Parsed request object
│   │   ├── HttpResponse.hpp / .cpp       # Response builder + toString()
│   │   └── HttpParser.hpp / .cpp         # Raw bytes → HttpRequest
│   ├── router/
│   │   └── Router.hpp / Router.cpp       # Method+path → handler dispatch
│   ├── threadpool/
│   │   └── ThreadPool.hpp / .cpp         # Worker threads + work queue
│   ├── utils/
│   │   └── Logger.hpp / Logger.cpp       # Timestamped console logger
│   └── main.cpp
├── tests/
├── benchmarks/
├── CMakeLists.txt
└── README.md
```

---

## Build

### Requirements

- Ubuntu 22.04 (or any Linux with epoll support)
- GCC 11+
- CMake 3.22+

### Steps

```bash
git clone https://github.com/yohanan400/webserver.git
cd webserver
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bin/webserver
```

For debug build with AddressSanitizer:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/bin/webserver
```

---

## Usage

### Adding a Route

```cpp
router.addRoute("GET", "/api/user", [](const HttpRequest& req, const std::shared_ptr<Socket>& socket) {
    HttpResponse response;
    response.setCode(200);
    response.setBody(R"({"message": "Hello!"})");
    response.setHeaders({
        {"Content-Type", "application/json"},
        {"Content-Length", std::to_string(response.getBody().size())}
    });
    const std::string res = response.toString();
    ::send(socket->getFd(), res.c_str(), res.length(), 0);
});
```

### Testing with curl

```bash
# Basic request
curl http://localhost:5555/api/user

# Verbose output (shows headers)
curl -v http://localhost:5555/api/user

# POST request with body
curl -X POST http://localhost:5555/api/user \
     -H "Content-Type: application/json" \
     -d '{"name": "John"}'
```

---

## Design Decisions

### RAII for Sockets
Every file descriptor is wrapped in a `Socket` object. The destructor calls `close()` automatically — no manual cleanup required, no leaks possible.

### Thread Pool with Condition Variable
Worker threads sleep via `std::condition_variable` when the queue is empty — zero CPU usage while idle. A `std::atomic<bool>` signals shutdown without a mutex.

### Hash-Map Router
Routes are stored as `"METHOD:path"` keys in `std::unordered_map`, giving O(1) lookup regardless of the number of registered routes.

### HTTP Parser
Parsing is split into three stages — request line, headers, body — each operating on a `std::string` slice of the raw input using `find` and `substr`. No regex, no external libraries.

---

## Concepts Demonstrated

| Concept | Where |
|---|---|
| Raw TCP sockets | `net/Socket`, `net/Acceptor` |
| RAII | `Socket` destructor, `unique_lock` |
| Thread pool | `threadpool/ThreadPool` |
| `std::mutex` + `std::condition_variable` | `ThreadPool` work queue |
| `std::atomic` | `ThreadPool::_stop` |
| Move semantics | `Socket(Socket&&)`, lambda captures |
| `shared_ptr` | Socket lifetime across threads |
| Hash map routing | `Router::_handlers_map` |
| HTTP/1.1 parsing | `HttpParser` |
| Design patterns | RAII, Reactor (Acceptor), Chain of Responsibility (planned Middleware) |

---

## Roadmap

- [ ] Static file server with `mmap`
- [ ] Middleware pipeline (logger, CORS, gzip)
- [ ] Keep-Alive connection management
- [ ] `epoll` non-blocking I/O
- [ ] TLS/HTTPS via OpenSSL
- [ ] Benchmarking vs nginx with `wrk`
- [ ] C++17 version with coroutines

---

## License

MIT
