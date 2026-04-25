#include "HttpRequest.h"
[[nodiscard]] std::string HttpRequest::get_method() const
{
    return _method;
}

void HttpRequest::set_method(const std::string& method)
{
    _method = method;
}

[[nodiscard]] std::string HttpRequest::get_path() const
{
    return _path;
}

void HttpRequest::set_path(const std::string& path)
{
    _path = path;
}

[[nodiscard]] std::string HttpRequest::get_query() const
{
    return _query;
}

void HttpRequest::set_query(const std::string& query)
{
    _query = query;
}

[[nodiscard]] std::unordered_map<std::string, std::string> HttpRequest::get_headers() const
{
    return _headers;
}

void HttpRequest::set_headers(const std::unordered_map<std::string, std::string>& headers)
{
    _headers = headers;
}

[[nodiscard]] std::string HttpRequest::get_body() const
{
    return _body;
}

void HttpRequest::set_body(const std::string& body)
{
    _body = body;
}

[[nodiscard]] std::string HttpRequest::get_version() const
{
    return _version;
}

void HttpRequest::set_version(const std::string& version)
{
    _version = version;
}