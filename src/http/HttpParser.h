#pragma once
#include "HttpRequest.h"

class HttpParser
{
public:
    HttpRequest parse(const std::string& raw);

private:
    HttpRequest _httpRequest;

    void parseRequestLine(const std::string& line);
    void parseHeaders(const std::string& headersSection);
    void parseBody(const std::string& body);
};
