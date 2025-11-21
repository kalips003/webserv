#include "webserv.hpp"

struct StatusEntry {
    int code;
    const char *message;
};

static const StatusEntry g_HttpStatusCode[] = {
// --- 1xx: Informational ---
    {100, "Continue"},
    {101, "Switching Protocols"},
    {102, "Processing"},
    {103, "Early Hints"},

// --- 2xx: Success ---
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritative Information"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {207, "Multi-Status"},
    {208, "Already Reported"},
    {226, "IM Used"},

// --- 3xx: Redirection ---
    {300, "Multiple Choices"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {305, "Use Proxy"},
    {307, "Temporary Redirect"},
    {308, "Permanent Redirect"},

// --- 4xx: Client Errors ---
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Timeout"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Precondition Failed"},
    {413, "Payload Too Large"},
    {414, "URI Too Long"},
    {415, "Unsupported Media Type"},
    {416, "Range Not Satisfiable"},
    {417, "Expectation Failed"},
    {418, "I'm a teapot"},
    {421, "Misdirected Request"},
    {422, "Unprocessable Entity"},
    {423, "Locked"},
    {424, "Failed Dependency"},
    {425, "Too Early"},
    {426, "Upgrade Required"},
    {428, "Precondition Required"},
    {429, "Too Many Requests"},
    {431, "Request Header Fields Too Large"},
    {451, "Unavailable For Legal Reasons"},

// --- 5xx: Server Errors ---
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Timeout"},
    {505, "HTTP Version Not Supported"},
    {506, "Variant Also Negotiates"},
    {507, "Insufficient Storage"},
    {508, "Loop Detected"},
    {510, "Not Extended"},
    {511, "Network Authentication Required"}
};

///////////////////////////////////////////////////////////////////////////////]
const std::map<int, std::string>&   http_status_map() {

    static std::map<int, std::string>   m;
    if (m.empty()) {

        const size_t n = sizeof(g_HttpStatusCode) / sizeof(g_HttpStatusCode[0]);
        for (size_t i = 0; i < n; i++)
            m[g_HttpStatusCode[i].code] = g_HttpStatusCode[i].message;

    }
    return m;
}

///////////////////////////////////////////////////////////////////////////////]
std::string   return_http_from_code(int code) {

    std::map<int, std::string>::const_iterator it = http_status_map().find(code);
    if (it != http_status_map().end())
        return it->second;
    return "";
}

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

int isHTTP_Request_HeadersValid(const std::string& s) {

    const std::vector<std::string>&    reqHeader = HTTP_Request_Headers_list();
    for (size_t i = 0; i < reqHeader.size(); ++i) {
        if (s == reqHeader[i])
            return static_cast<int>(i);
    }
    return -1;
}

static const char* g_http_response_headers[] = {
    "Accept-Ranges",
    "Age",
    "Allow",
    "Cache-Control",
    "Connection",
    "Content-Encoding",
    "Content-Language",
    "Content-Length",
    "Content-Location",
    "Content-Disposition",
    "Content-Type",
    "Date",
    "ETag",
    "Expires",
    "Last-Modified",
    "Location",
    "Pragma",
    "Retry-After",
    "Server",
    "Set-Cookie",
    "Strict-Transport-Security",
    "Trailer",
    "Transfer-Encoding",
    "Upgrade",
    "Vary",
    "Via",
    "Warning",
    "WWW-Authenticate"
};

///////////////////////////////////////////////////////////////////////////////]
const std::vector<std::string>& HTTP_Response_Headers_list() {

    static std::vector<std::string> s;
    if (s.empty()) {
    
        const size_t n = sizeof(g_http_response_headers) / sizeof(g_http_response_headers[0]);
        for (size_t i = 0; i < n; i++)
            s.push_back(g_http_response_headers[i]);
    }
    return s;
}
