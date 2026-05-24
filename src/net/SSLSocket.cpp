#include "SSLSocket.h"

#include <stdexcept>

#include "utils/Logger.h"

SSL_CTX * SSLSocket::_ctx = nullptr;

SSLSocket::SSLSocket(const int fd, const std::string& ip) : Socket(fd, ip)
{
    _ssl = SSL_new(_ctx);
    SSL_set_fd(_ssl, _fd);

    int ret = SSL_accept(_ssl);
    if (ret <= 0)
        throw std::runtime_error("SSL handshake failed");
}

SSLSocket::SSLSocket(SSLSocket&& other) noexcept : Socket(std::move(other)), _ssl(other._ssl)
{
    other._ssl = nullptr;
}

ssize_t SSLSocket::send(const void *buf, const size_t len, int flags)
{
    (void)flags;
    return SSL_write(_ssl, buf, static_cast<int>(len));
}

ssize_t SSLSocket::recv(void *buf, const size_t len, int flags)
{
    (void)flags;
    const int bytes_read = SSL_read(_ssl, buf, static_cast<int>(len));

    if (bytes_read >= 0) return bytes_read;

    const int err = SSL_get_error(_ssl, bytes_read);

    if (err == SSL_ERROR_WANT_READ) {
        Logger::error("Waiting for HTTP request (WANT_READ)...\n");
        return 0;
    }

    if (err == SSL_ERROR_ZERO_RETURN) {
         Logger::error("Client closed connection.\n");
    }
    else {
        Logger::error("Actual SSL read error code: " + std::to_string(err) + "\n");
    }
    return -1;
}

void SSLSocket::initCTX(const std::string& cert, const std::string& key)
{
    SSL_library_init();
    SSL_load_error_strings();

    _ctx = SSL_CTX_new(TLS_server_method());
    if (!_ctx)
        throw std::runtime_error("Failed to create SSL context");

    if (SSL_CTX_use_certificate_file(_ctx, cert.c_str(), SSL_FILETYPE_PEM) <= 0)
        throw std::runtime_error("Failed to load certificate");

    if (SSL_CTX_use_PrivateKey_file(_ctx, key.c_str(), SSL_FILETYPE_PEM) <= 0)
        throw std::runtime_error("Failed to load private key");
}

void SSLSocket::cleanupCTX()
{
    SSL_CTX_free(_ctx);
    _ctx = nullptr;
}

SSLSocket::~SSLSocket()
{
    if (_ssl)
    {
        SSL_shutdown(_ssl);
        SSL_free(_ssl);
        _ssl = nullptr;
    }
}