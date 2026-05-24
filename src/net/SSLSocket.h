#pragma once
#include "Socket.h"
#include <openssl/ssl.h>

class SSLSocket : public Socket
{
public:
    SSLSocket(int fd, const std::string& ip);
    SSLSocket(SSLSocket&& other) noexcept ;
    ssize_t send(const void *buf, size_t len, int flags) override;
    ssize_t recv(void *buf, size_t len, int flags) override;
    static void initCTX(const std::string& cert, const std::string& key);
    static void cleanupCTX();
    ~SSLSocket() override;

private:
    static SSL_CTX *_ctx;
    SSL *_ssl;
};
