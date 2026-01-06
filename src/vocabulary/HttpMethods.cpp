#include "HttpMethods.hpp"

///////////////////////////////////////////////////////////////////////////////]
// WebDAV += {PROPFIND, PROPPATCH, MKCOL, COPY, MOVE, LOCK, UNLOCK}
static const char* g_HttpMethods[] = {
    "GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS", "CONNECT", "TRACE"
};

///////////////////////////////////////////////////////////////////////////////]
const std::vector<std::string>& http_method_list() {

    static std::vector<std::string> s;
    if (s.empty()) {
    
        const size_t n = sizeof(g_HttpMethods) / sizeof(g_HttpMethods[0]);
        for (size_t i = 0; i < n; i++)
            s.push_back(g_HttpMethods[i]);
    }
    return s;
}

///////////////////////////////////////////////////////////////////////////////]
HttpMethod isMethodValid(const std::string& s) {

    size_t i = -1;
    while (++i < http_method_list().size()) {
        if (s == http_method_list()[i])
            return static_cast<HttpMethod>(i);
    }
    return INVALID;
}