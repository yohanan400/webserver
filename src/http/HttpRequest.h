#pragma once
#include <string>
#include <unordered_map>

class HttpRequest {
public:
    HttpRequest() = default;

    [[nodiscard]] std::string getMethod() const;
    void setMethod(const std::string& method);

    [[nodiscard]] std::string getPath() const;
    void setPath(const std::string& path);

    [[nodiscard]] std::string getQuery() const;
    void setQuery(const std::string& query);

    [[nodiscard]] std::unordered_map<std::string, std::string> getHeaders() const;
    void setHeaders(const std::unordered_map<std::string, std::string>& headers);

    [[nodiscard]] std::string getBody() const;
    void setBody(const std::string& body);

    [[nodiscard]] std::string getVersion() const;
    void setVersion(const std::string& version);

private:
    std::string _method;
    std::string _path;
    std::string _query;
    std::unordered_map<std::string, std::string> _headers;
    std::string _body;
    std::string _version;
};
