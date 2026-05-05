[To be honest.. I was too lazy to write all this // so this is claude made ;)]

# HTTP Web Server in C++17

A fully functional HTTP/1.1 web server written from scratch in C++17, without any external server libraries. Built as a portfolio project demonstrating advanced systems programming concepts including multithreading, network programming, middleware design patterns, and HTTP protocol implementation.

---

## Features

- **TCP Server** — raw socket programming with `epoll`-ready architecture
- **Thread Pool** — concurrent request handling with work queue and graceful shutdown
- **HTTP Parser** — state-based parsing of HTTP/1.1 requests including headers, query strings, and body
- **Router** — fast hash-map based routing with support for multiple HTTP methods and static file serving
- **HTTP Response** — structured response builder with automatic status text resolution
- **Middleware Pipeline** — Chain of Responsibility pattern with recursive lambda dispatch
- **Logger Middleware** — per-request timing using `std::chrono::steady_clock`
- **CORS Middleware** — preflight `OPTIONS` handling and `Access-Control-*` headers
- **Rate Limiter** — sliding-window IP-based rate limiting with `std::mutex` thread safety
- **Static File Server** — serves files from `src/static/` with MIME type detection
- **RAII throughout** — all resources (sockets, threads) managed automatically via C++ destructors

---

## Architecture

```
Client Request
      ↓
  Acceptor Thread        (net/Acceptor)       — accept() + extract client IP
      ↓
  Thread Pool            (threadpool/ThreadPool)
      ↓
  HTTP Parser            (http/HttpParser)
      ↓
  Middleware Pipeline    (middleware/Middleware)
      ├── Rate Limiter   (utils/RateLimiter)  — 429 if exceeded
      ├── CORS           — OPTIONS early return, Access-Control headers
      └── Logger         — method + path + duration
      ↓
  Router                 (router/Router)       — exact match or /static/ prefix
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
│   │   ├── Socket.hpp / Socket.cpp         # RAII TCP socket wrapper (fd + client IP)
│   │   └── Acceptor.hpp / Acceptor.cpp     # bind, listen, accept
│   ├── http/
│   │   ├── HttpRequest.hpp / .cpp          # Parsed request object
│   │   ├── HttpResponse.hpp / .cpp         # Response builder + toString()
│   │   └── HttpParser.hpp / .cpp           # Raw bytes → HttpRequest
│   ├── router/
│   │   └── Router.hpp / Router.cpp         # Method+path → handler dispatch
│   ├── middleware/
│   │   └── Middleware.hpp / Middleware.cpp  # Pipeline + recursive Next dispatch
│   ├── threadpool/
│   │   └── ThreadPool.hpp / .cpp           # Worker threads + work queue
│   ├── utils/
│   │   ├── Logger.hpp / Logger.cpp         # Timestamped console logger
│   │   └── RateLimiter.hpp / .cpp          # IP-based sliding window rate limiter
│   ├── static/                             # Static files served under /static/*
│   └── main.cpp
├── tests/
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

> **Note:** Set the working directory to `src/` so relative paths (e.g. `static/`) resolve correctly.  
> In CLion: Run/Debug Configurations → Working directory → `.../webserver/src`

For debug build:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

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
    // runs before handler
    next();
    // runs after handler
});
```

### Static Files

Place any file under `src/static/` and access it via:

```
GET /static/filename.ext
```

Supported MIME types: `html`, `css`, `js`, `json`, `xml`, `txt`, `png`, `jpg`, `jpeg`, `gif`, `svg`, `pdf`, `zip`, `mp3`, `mp4`.

### Testing with curl

```bash
# Basic request
curl http://localhost:5555/api/user

# Verbose (shows response headers)
curl -v http://localhost:5555/api/user

# Static file
curl http://localhost:5555/static/index.html

# CORS preflight
curl -X OPTIONS http://localhost:5555/api/user -v

# Trigger rate limiter (run 101 times)
for i in $(seq 1 101); do curl -s http://localhost:5555/api/user; done
```

---

## Design Decisions

### RAII for Sockets
Every file descriptor is wrapped in a `Socket` object that also stores the client IP. The destructor calls `close()` automatically — no manual cleanup required.

### Thread Pool with Condition Variable
Worker threads sleep via `std::condition_variable` when the queue is empty — zero CPU usage while idle. A `std::atomic<bool>` signals shutdown without a mutex.

### Hash-Map Router
Routes are stored as `"METHOD:path"` keys in `std::unordered_map`, giving O(1) lookup. Static file requests fall through to a `/static/` prefix check.

### Middleware Pipeline
Middlewares are stored as a `std::vector<std::function<...>>`. Dispatch uses a recursive lambda with an index — each middleware calls `next()` to advance the chain. This is the Chain of Responsibility pattern.

### Rate Limiter
Uses a sliding window: each IP maps to `{count, first_request_time}`. On each request, if the time window has expired the counter resets; otherwise it increments. Protected by `std::mutex` + `std::lock_guard` for thread safety.

### CORS
The CORS middleware handles `OPTIONS` preflight with an early return (204, no `next()` call) and adds `Access-Control-*` headers to all other responses before passing to the next middleware.

---

## Concepts Demonstrated

| Concept | Where |
|---|---|
| Raw TCP sockets | `net/Socket`, `net/Acceptor` |
| RAII | `Socket` destructor, `lock_guard` |
| Thread pool | `threadpool/ThreadPool` |
| `std::mutex` + `std::condition_variable` | `ThreadPool` work queue |
| `std::mutex` + `std::lock_guard` | `RateLimiter` |
| `std::atomic` | `ThreadPool::_stop` |
| Move semantics | `Socket(Socket&&)`, lambda captures |
| `shared_ptr` | Socket lifetime across threads |
| Hash map routing | `Router::_handlers_map` |
| HTTP/1.1 parsing | `HttpParser` |
| Chain of Responsibility | `MiddlewarePipeline` |
| Recursive lambda | `MiddlewarePipeline::execute` |
| `std::chrono::steady_clock` | Logger middleware timing |
| MIME type detection | Static file handler |
| Design patterns | RAII, Reactor (Acceptor), Chain of Responsibility (Middleware) |

---

## Roadmap

- [x] TCP Server + Thread Pool
- [x] HTTP Parser + Router
- [x] Middleware Pipeline
- [x] Logger / CORS / Rate Limiter middlewares
- [x] Static File Server
- [ ] Keep-Alive connection management
- [ ] `epoll` non-blocking I/O
- [ ] TLS/HTTPS via OpenSSL
- [ ] Benchmarking vs nginx with `wrk`

---

## License

MIT
