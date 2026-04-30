#include "HttpParser.h"

#include <stdexcept>

HttpRequest HttpParser::parse(const std::string& raw)
{
    size_t first_pos = raw.find("\r\n");
    size_t last_pos = raw.find("\r\n\r\n");

    constexpr size_t CRLF = 2;
    constexpr size_t CRLFCRLF = 4;

    if (first_pos == std::string::npos || last_pos == std::string::npos)
    {
        throw std::runtime_error("Invalid HTTP request!");
    }

    parseRequestLine(raw.substr(0, first_pos));
    parseHeaders(raw.substr(first_pos + CRLF, last_pos - (first_pos + CRLF)));
    parseBody(raw.substr(last_pos + CRLFCRLF));

    return _httpRequest;
}

void HttpParser::parseRequestLine(const std::string& line)
{
    const size_t end_method_pos = line.find(' ');
    const size_t end_path_pos = line.find('?');
    const size_t end_query_pos = line.find_last_of(' ');

    if (end_method_pos == std::string::npos || end_query_pos == std::string::npos)
    {
        throw std::runtime_error("Invalid HTTP request line!");
    }

    _httpRequest.setMethod(line.substr(0, end_method_pos));

    if (end_path_pos != std::string::npos)
    {
        _httpRequest.setPath(line.substr(end_method_pos + 1, end_path_pos - (end_method_pos + 1)));
        _httpRequest.setQuery(line.substr(end_path_pos + 1, end_query_pos - (end_path_pos + 1)));
    }
    else
    {
        _httpRequest.setPath(line.substr(end_method_pos + 1, end_query_pos - (end_method_pos + 1)));
    }

    _httpRequest.setVersion(line.substr(end_query_pos + 1));
}

void HttpParser::parseHeaders(const std::string& headersSection)
{
    if (!headersSection.empty())
    {
        std::unordered_map<std::string, std::string> headers;

        constexpr size_t CRLF = 2;
        size_t crlf_pos = 0;
        size_t next_crlf_pos = headersSection.find("\r\n");

        std::string key;
        std::string value;
        size_t colon_pos;

        do
        {
            std::string header = headersSection.substr(crlf_pos, next_crlf_pos - crlf_pos);

            colon_pos = header.find(':');
            key = header.substr(0, colon_pos);
            value = header.substr(colon_pos + 2);

            headers.insert({key, value});

            crlf_pos = next_crlf_pos;
            next_crlf_pos = headersSection.find("\r\n", crlf_pos + CRLF);
        }
        while (next_crlf_pos != std::string::npos);

        _httpRequest.setHeaders(headers);
    }
}

void HttpParser::parseBody(const std::string& body)
{
    if (!body.empty())
    {
        _httpRequest.setBody(body);
    }
}
