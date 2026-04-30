#pragma once
#include <string>
#include <unordered_map>

class HttpResponse
{
public:
    HttpResponse();

    std::string toString() const;

    [[nodiscard]] size_t getCode() const;
    void setCode(size_t code);

    [[nodiscard]] std::string getCodeString() const;

    [[nodiscard]] std::string getVersion() const;
    void setVersion(const std::string& version);

    [[nodiscard]] std::string getBody() const;
    void setBody(const std::string& body);

    [[nodiscard]] std::unordered_map<std::string, std::string> getHeaders() const;
    void setHeaders(const std::unordered_map<std::string, std::string>& headers);

private:
    size_t _code = 0;
    std::string _code_string;
    std::unordered_map<size_t, std::string> _status_codes;
    std::string _version;
    std::string _body;
    std::unordered_map<std::string, std::string> _headers;
};
