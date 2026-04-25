#pragma once
#include <string>
#include <unordered_map>

class HttpRequest {
public:
    HttpRequest() = default;

    [[nodiscard]] std::string get_method() const;
    void set_method(const std::string& method);

    [[nodiscard]] std::string get_path() const;
    void set_path(const std::string& path);

    [[nodiscard]] std::string get_query() const;
    void set_query(const std::string& query);

    [[nodiscard]] std::unordered_map<std::string, std::string> get_headers() const;
    void set_headers(const std::unordered_map<std::string, std::string>& headers);

    [[nodiscard]] std::string get_body() const;
    void set_body(const std::string& body);

    [[nodiscard]] std::string get_version() const;
    void set_version(const std::string& version);

private:
    std::string _method;
    std::string _path;
    std::string _query;
    std::unordered_map<std::string, std::string> _headers;
    std::string _body;
    std::string _version;
};
