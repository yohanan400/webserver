[To be honest.. I was too lazy to write all this // so this is claude made ;)]

# HTTP Web Server in C++17

> A fully functional HTTP/1.1 web server written from scratch in C++17, without any external server libraries. Built as a portfolio project demonstrating advanced systems programming concepts including multithreading, network programming, TLS encryption, middleware design patterns, and HTTP protocol implementation.

---

## Features

- **TCP Server** — raw socket programming with `epoll` Edge Triggered non-blocking I/O
- **TLS/HTTPS** — OpenSSL integration via `SSLSocket` inheriting from `Socket`, with full handshake and encrypted I/O
- **epoll ET** — scalable I/O multiplexing with Edge Triggered mode and non-blocking sockets
- **Thread Pool** — concurrent request handling with work queue and graceful shutdown
- **HTTP Parser** — state-based parsing of HTTP/1.1 requests including headers, query strings, and body
- **Router** — fast hash-map based routing with support for multiple HTTP methods and static file serving
- **HTTP Response** — structured response builder with automatic status text resolution
- **Middleware Pipeline** — Chain of Responsibility pattern with recursive lambda dispatch
- **Logger Middleware** — per-request timing using `std::chrono::steady_clock`
- **CORS Middleware** — preflight `OPTIONS` handling and `Access-Control-*` headers
- **Rate Limiter** — sliding-window IP-based rate limiting with `std::mutex` thread safety
- **Static File Server** — serves files from `src/static/` with MIME type detection
- **Log Levels** — `DEBUG / INFO / ERROR` with runtime `setLogLevel()` control
- **RAII throughout** — all resources managed automatically via C++ destructors

---

## Architecture

```
Client Request (HTTPS)
      ↓
  Acceptor              (net/Acceptor)        — accept() + extract client IP
      ↓
  SSLSocket             (net/SSLSocket)        — TLS handshake via OpenSSL
      ↓
  EventLoop             (EventLoop)            — epoll ET, manages connections map
      ↓
  Thread Pool           (threadpool/ThreadPool)
      ↓
  Buffer Accumulation   — accumulate until \r\n\r\n (handles partial reads)
      ↓
  HTTP Parser           (http/HttpParser)
      ↓
  Middleware Pipeline   (middleware/Middleware)
      ├── Rate Limiter  (utils/RateLimiter)   — 429 if exceeded
      ├── CORS          — OPTIONS early return, Access-Control headers
      └── Logger        — method + path + duration
      ↓
  Router                (router/Router)        — exact match or /static/ prefix
      ↓
  Handler → HttpResponse (http/HttpResponse)
      ↓
  SSLSocket::send()     — encrypted response → Client
```

---

## Project Structure

```
webserver/
├── src/
│   ├── net/
│   │   ├── Socket.h / Socket.cpp           # RAII TCP socket wrapper
│   │   ├── SSLSocket.h / SSLSocket.cpp     # TLS socket + buffer accumulation
│   │   └── Acceptor.h / Acceptor.cpp       # bind, listen, accept → SSLSocket
│   ├── http/
│   │   ├── HttpRequest.h / .cpp
│   │   ├── HttpResponse.h / .cpp
│   │   └── HttpParser.h / .cpp
│   ├── router/
│   │   └── Router.h / Router.cpp
│   ├── middleware/
│   │   └── Middleware.h / Middleware.cpp
│   ├── threadpool/
│   │   └── ThreadPool.h / .cpp
│   ├── utils/
│   │   ├── Logger.h / Logger.cpp           # Timestamped logger with log levels
│   │   └── RateLimiter.h / .cpp
│   ├── EventLoop.h / EventLoop.cpp         # epoll ET event loop
│   ├── static/
│   └── main.cpp
├── tests/
├── CMakeLists.txt
└── README.md
```

---

## Build

### Requirements

- Linux (epoll support required)
- GCC 11+
- CMake 3.22+
- OpenSSL (`libssl-dev`)

```bash
sudo apt install libssl-dev
```

### TLS Certificate (development)

```bash
openssl req -x509 -newkey rsa:4096 -keyout src/key.pem -out src/cert.pem -days 365 -nodes
```

### Steps

```bash
git clone https://github.com/yohanan400/webserver.git
cd webserver
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bin/webserver
```

> **Note:** Set the working directory to `src/` so relative paths resolve correctly.

---

## Usage

### Adding a Route

```cpp
router.addRoute("GET", "/api/user",
    [](const HttpRequest& req, HttpResponse& response, const std::shared_ptr<Socket>& socket) {
        response.setCode(200);
        response.setBody(R"({"message": "Hello!"})");
        response.setHeaders({
            {"Content-Type", "application/json"},
            {"Content-Length", std::to_string(response.getBody().size())}
        });
    });
```

### Adding a Middleware

```cpp
mpl.use([](const HttpRequest& req, HttpResponse& res,
           const std::shared_ptr<Socket>& socket, const Next& next) {
    next();
});
```

### Log Level Control

```cpp
Logger::setLogLevel(LogLevel::ERROR); // production
Logger::setLogLevel(LogLevel::DEBUG); // development
```

### Testing with curl

```bash
curl -k https://localhost:5555/api/user
curl -kv https://localhost:5555/api/user
curl -k https://localhost:5555/static/index.html
curl -k -X OPTIONS https://localhost:5555/api/user -v
for i in $(seq 1 101); do curl -sk https://localhost:5555/api/user; done
```

---

## Design Decisions

### epoll Edge Triggered
The `EventLoop` class manages an epoll instance with ET mode. Each new connection is registered with `EPOLLIN | EPOLLET` and set to non-blocking via `fcntl`. The event loop dispatches ready connections to the thread pool. `EPOLLHUP`/`EPOLLERR` are caught before dispatch for fast cleanup.

### Buffer Accumulation
Each `SSLSocket` holds a `_buffer` string. The read loop accumulates data until `\r\n\r\n` is found, handling cases where a request spans multiple `recv()` calls (common with large requests or slow networks). On `EAGAIN` without a complete header — the connection is paused and waits for the next epoll trigger.

### Thread Safety in EventLoop
`_connections` (an `unordered_map<int, shared_ptr<SSLSocket>>`) is accessed from both the event loop thread and worker threads. A `std::mutex` with minimal scope protects all accesses. `find()` is used instead of `operator[]` to avoid creating empty entries.

### Log Levels
`enum class LogLevel { DEBUG, INFO, ERROR }` with a static `_logLevel` field. Each log call checks the current level before printing — zero overhead in production when set to `ERROR`.

### TLS via Inheritance
`SSLSocket` inherits from `Socket` and overrides `send()`/`recv()` with `SSL_write()`/`SSL_read()`. The rest of the codebase uses `std::shared_ptr<Socket>`.

### Thread Pool with Condition Variable
Worker threads sleep via `std::condition_variable` when the queue is empty. A `std::atomic<bool>` signals shutdown without a mutex.

### Hash-Map Router
Routes stored as `"METHOD:path"` keys in `std::unordered_map` — O(1) lookup.

### Middleware Pipeline
`std::vector` of functions with recursive lambda dispatch — Chain of Responsibility pattern.

---

## Concepts Demonstrated

| Concept | Where |
|---|---|
| Raw TCP sockets | `net/Socket`, `net/Acceptor` |
| TLS/SSL | `net/SSLSocket`, OpenSSL |
| epoll Edge Triggered | `EventLoop` |
| Non-blocking I/O | `fcntl O_NONBLOCK`, `EAGAIN` handling |
| Buffer accumulation | `SSLSocket::_buffer` |
| Thread pool | `threadpool/ThreadPool` |
| `std::mutex` + minimal scope locking | `EventLoop::_connections` |
| `std::condition_variable` | `ThreadPool` work queue |
| `std::atomic` | `ThreadPool::_stop` |
| Move semantics | `Socket`, `SSLSocket`, lambda captures |
| `shared_ptr` | Socket lifetime across threads |
| Inheritance + virtual dispatch | `Socket` → `SSLSocket` |
| RAII | Destructors throughout |
| Hash map routing | `Router` |
| Chain of Responsibility | `MiddlewarePipeline` |
| Recursive lambda | `MiddlewarePipeline::execute` |
| Log levels | `Logger` |

---

## Roadmap

- [x] TCP Server + Thread Pool
- [x] HTTP Parser + Router
- [x] Middleware Pipeline
- [x] Logger / CORS / Rate Limiter middlewares
- [x] Static File Server
- [x] TLS/HTTPS via OpenSSL
- [x] epoll Edge Triggered non-blocking I/O
- [x] Buffer accumulation for partial reads
- [x] Log levels
- [ ] Benchmarking vs nginx with `wrk`
- [ ] Microservices architecture (URL Shortener + Click Analytics)

---

## License

MIT
