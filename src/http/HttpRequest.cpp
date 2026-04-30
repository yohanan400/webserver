#include "HttpRequest.h"

[[nodiscard]] std::string HttpRequest::getMethod() const
{
    return _method;
}

void HttpRequest::setMethod(const std::string& method)
{
    _method = method;
}

[[nodiscard]] std::string HttpRequest::getPath() const
{
    return _path;
}

void HttpRequest::setPath(const std::string& path)
{
    _path = path;
}

[[nodiscard]] std::string HttpRequest::getQuery() const
{
    return _query;
}

void HttpRequest::setQuery(const std::string& query)
{
    _query = query;
}

[[nodiscard]] std::unordered_map<std::string, std::string> HttpRequest::getHeaders() const
{
    return _headers;
}

void HttpRequest::setHeaders(const std::unordered_map<std::string, std::string>& headers)
{
    _headers = headers;
}

[[nodiscard]] std::string HttpRequest::getBody() const
{
    return _body;
}

void HttpRequest::setBody(const std::string& body)
{
    _body = body;
}

[[nodiscard]] std::string HttpRequest::getVersion() const
{
    return _version;
}

void HttpRequest::setVersion(const std::string& version)
{
    _version = version;
}
