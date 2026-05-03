#include "HttpResponse.h"

#include <stdexcept>

HttpResponse::HttpResponse()
{
    _status_codes = {
        {200, "OK"},
        {201, "Created"},
        {204, "No Content"},
        {301, "Moved Permanently"},
        {302, "Found"},
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {403, "Forbidden"},
        {429, "Too Many Requests"},
        {404, "Not Found"},
        {500, "Internal Server Error"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"}
    };

    _version = "HTTP/1.1";
}

std::string HttpResponse::toString() const
{
    std::string headers;

    for (const auto& header : _headers)
    {
        headers += header.first + ": " + header.second + "\r\n";
    }

    return _version + " " + std::to_string(_code) + " " + _code_string + "\r\n"\
        + headers + "\r\n" + _body;
}

[[nodiscard]] size_t HttpResponse::getCode() const
{
    return _code;
}

void HttpResponse::setCode(const size_t code)
{
    if (_status_codes.find(code) != _status_codes.end())
    {
        _code = code;
        _code_string = "Http Code: " + std::to_string(code) + " - " + _status_codes[code];
    }
    else throw std::runtime_error("HttpResponse::setCode Unsupported status code.");
}

[[nodiscard]] std::string HttpResponse::getCodeString() const
{
    return _code_string;
}

[[nodiscard]] std::string HttpResponse::getVersion() const
{
    return _version;
}

void HttpResponse::setVersion(const std::string& version)
{
    _version = version;
}

[[nodiscard]] std::string HttpResponse::getBody() const
{
    return _body;
}

void HttpResponse::setBody(const std::string& body)
{
    _body = body;
}

[[nodiscard]] std::unordered_map<std::string, std::string> HttpResponse::getHeaders() const
{
    return _headers;
}

void HttpResponse::setHeaders(const std::unordered_map<std::string, std::string>& headers)
{
    _headers = headers;
}
