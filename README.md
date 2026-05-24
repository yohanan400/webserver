[To be honest.. I was too lazy to write all this // so this is claude made ;)]

# HTTP Web Server in C++17

A fully functional HTTP/1.1 web server written from scratch in C++17, without any external server libraries. Built as a portfolio project demonstrating advanced systems programming concepts including multithreading, network programming, TLS encryption, middleware design patterns, and HTTP protocol implementation.

---

## Features

- **TCP Server** — raw socket programming with `epoll`-ready architecture
- **TLS/HTTPS** — OpenSSL integration via `SSLSocket` inheriting from `Socket`, with full handshake and encrypted I/O
- **Thread Pool** — concurrent request handling with work queue and graceful shutdown
- **HTTP Parser** — state-based parsing of HTTP/1.1 requests including headers, query strings, and body
- **Router** — fast hash-map based routing with support for multiple HTTP methods and static file serving
- **HTTP Response** — structured response builder with automatic status text resolution
- **Middleware Pipeline** — Chain of Responsibility pattern with recursive lambda dispatch
- **Logger Middleware** — per-request timing using `std::chrono::steady_clock`
- **CORS Middleware** — preflight `OPTIONS` handling and `Access-Control-*` headers
- **Rate Limiter** — sliding-window IP-based rate limiting with `std::mutex` thread safety
- **Static File Server** — serves files from `src/static/` with MIME type detection
- **Keep-Alive** — persistent connections with configurable timeout via `SO_RCVTIMEO`
- **RAII throughout** — all resources (sockets, threads, SSL objects) managed automatically via C++ destructors

---

## Architecture

```
Client Request (HTTPS)
      ↓
  Acceptor Thread        (net/Acceptor)       — accept() + extract client IP
      ↓
  SSLSocket              (net/SSLSocket)       — TLS handshake via OpenSSL
      ↓
  Thread Pool            (threadpool/ThreadPool)
      ↓
  HTTP Parser            (http/HttpParser)
      ↓
  Keep-Alive Loop        — do/while with SO_RCVTIMEO timeout
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
  SSLSocket::send()      — encrypted response → Client
```

---

## Project Structure

```
webserver/
├── src/
│   ├── net/
│   │   ├── Socket.h / Socket.cpp           # RAII TCP socket wrapper (fd + client IP), virtual send/recv
│   │   ├── SSLSocket.h / SSLSocket.cpp     # TLS socket inheriting Socket, OpenSSL integration
│   │   └── Acceptor.h / Acceptor.cpp       # bind, listen, accept → SSLSocket
│   ├── http/
│   │   ├── HttpRequest.h / .cpp            # Parsed request object
│   │   ├── HttpResponse.h / .cpp           # Response builder + toString()
│   │   └── HttpParser.h / .cpp             # Raw bytes → HttpRequest
│   ├── router/
│   │   └── Router.h / Router.cpp           # Method+path → handler dispatch
│   ├── middleware/
│   │   └── Middleware.h / Middleware.cpp   # Pipeline + recursive Next dispatch
│   ├── threadpool/
│   │   └── ThreadPool.h / .cpp             # Worker threads + work queue
│   ├── utils/
│   │   ├── Logger.h / Logger.cpp           # Timestamped console logger
│   │   └── RateLimiter.h / .cpp            # IP-based sliding window rate limiter
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

> **Note:** Set the working directory to `src/` so relative paths (`static/`, `cert.pem`, `key.pem`) resolve correctly.  
> In CLion: Run/Debug Configurations → Working directory → `.../webserver/src`

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
# HTTPS request (skip certificate verification for self-signed)
curl -k https://localhost:5555/api/user

# Verbose (shows TLS handshake + headers)
curl -kv https://localhost:5555/api/user

# Static file
curl -k https://localhost:5555/static/index.html

# CORS preflight
curl -k -X OPTIONS https://localhost:5555/api/user -v

# Trigger rate limiter (run 101 times)
for i in $(seq 1 101); do curl -sk https://localhost:5555/api/user; done
```

---

## Design Decisions

### TLS via Inheritance
`SSLSocket` inherits from `Socket` and overrides the virtual `send()`/`recv()` methods with `SSL_write()`/`SSL_read()`. The rest of the codebase uses `std::shared_ptr<Socket>` — no changes needed anywhere else. `SSL_CTX` is a static member shared across all connections; each connection gets its own `SSL*` object.

### RAII for Sockets and SSL
`Socket` destructor calls `close(_fd)`. `SSLSocket` destructor calls `SSL_shutdown()` + `SSL_free()` before the base destructor closes the fd. No manual cleanup anywhere.

### Keep-Alive with SO_RCVTIMEO
Each client connection runs a `do/while` loop in its thread. `SO_RCVTIMEO` is set on the socket so `recv()` returns after 5 seconds of inactivity with `errno == EAGAIN`, cleanly closing the connection.

### Thread Pool with Condition Variable
Worker threads sleep via `std::condition_variable` when the queue is empty — zero CPU usage while idle. A `std::atomic<bool>` signals shutdown without a mutex.

### Hash-Map Router
Routes are stored as `"METHOD:path"` keys in `std::unordered_map`, giving O(1) lookup. Static file requests fall through to a `/static/` prefix check.

### Middleware Pipeline
Middlewares are stored as a `std::vector<std::function<...>>`. Dispatch uses a recursive lambda with an index — each middleware calls `next()` to advance the chain. This is the Chain of Responsibility pattern.

### Rate Limiter
Uses a sliding window: each IP maps to `{count, window_start_time}`. On each request, if the time window has expired the counter resets; otherwise it increments. Protected by `std::mutex` + `std::lock_guard` for thread safety.

### CORS
The CORS middleware handles `OPTIONS` preflight with an early return (204, no `next()` call) and adds `Access-Control-*` headers to all other responses before passing to the next middleware.

---

## Concepts Demonstrated

| Concept | Where |
|---|---|
| Raw TCP sockets | `net/Socket`, `net/Acceptor` |
| TLS/SSL | `net/SSLSocket`, OpenSSL |
| Inheritance + virtual dispatch | `Socket` → `SSLSocket` |
| RAII | `Socket` destructor, `SSLSocket` destructor, `lock_guard` |
| Thread pool | `threadpool/ThreadPool` |
| `std::mutex` + `std::condition_variable` | `ThreadPool` work queue |
| `std::mutex` + `std::lock_guard` | `RateLimiter` |
| `std::atomic` | `ThreadPool::_stop` |
| Move semantics | `Socket(Socket&&)`, `SSLSocket(SSLSocket&&)`, lambda captures |
| `shared_ptr` | Socket lifetime across threads |
| `SO_RCVTIMEO` | Keep-Alive timeout |
| Hash map routing | `Router::_handlers_map` |
| HTTP/1.1 parsing | `HttpParser` |
| Chain of Responsibility | `MiddlewarePipeline` |
| Recursive lambda | `MiddlewarePipeline::execute` |
| `std::chrono::steady_clock` | Logger middleware timing |
| MIME type detection | Static file handler |
| `setsockopt` | `SO_REUSEADDR`, `SO_RCVTIMEO` |

---

## Roadmap

- [x] TCP Server + Thread Pool
- [x] HTTP Parser + Router
- [x] Middleware Pipeline
- [x] Logger / CORS / Rate Limiter middlewares
- [x] Static File Server
- [x] Keep-Alive with timeout (`SO_RCVTIMEO`)
- [x] TLS/HTTPS via OpenSSL
- [ ] `epoll` non-blocking I/O
- [ ] Benchmarking vs nginx with `wrk`

---

## License

MIT
