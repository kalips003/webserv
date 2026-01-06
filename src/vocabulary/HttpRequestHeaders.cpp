#include "HttpRequestHeaders.hpp"

///////////////////////////////////////////////////////////////////////////////]
static const char* g_http_request_headers[] = {
    "accept",
    "accept-charset",
    "accept-encoding",
    "accept-language",
    "authorization",
    "cache-control",
    "connection",
    "content-length",
    "content-type",
    "cookie",
    "date",
    "expect",
    "from",
    "host",
    "if-match",
    "if-modified-since",
    "if-none-match",
    "if-range",
    "if-unmodified-since",
    "max-forwards",
    "origin",
    "pragma",
    "proxy-authorization",
    "range",
    "referer",
    "te",
    "upgrade",
    "user-agent",
    "via",
    "warning"
};

///////////////////////////////////////////////////////////////////////////////]
const std::vector<std::string>& HTTP_Request_Headers_list() {

    static std::vector<std::string> s;
    if (s.empty()) {
    
        const size_t n = sizeof(g_http_request_headers) / sizeof(g_http_request_headers[0]);
        for (size_t i = 0; i < n; i++)
            s.push_back(g_http_request_headers[i]);
    }
    return s;
}

///////////////////////////////////////////////////////////////////////////////]
int isHTTP_Request_HeadersValid(const std::string& s) {

    const std::vector<std::string>&    reqHeader = HTTP_Request_Headers_list();
    for (size_t i = 0; i < reqHeader.size(); ++i) {
        if (s == reqHeader[i])
            return static_cast<int>(i);
    }
    return -1;
}