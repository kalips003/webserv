#include "HttpAnswerHeaders.hpp"

///////////////////////////////////////////////////////////////////////////////]
static const char* g_http_answer_headers[] = {
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
const std::vector<std::string>& HTTP_Answer_Headers_list() {

    static std::vector<std::string> s;
    if (s.empty()) {
    
        const size_t n = sizeof(g_http_answer_headers) / sizeof(g_http_answer_headers[0]);
        for (size_t i = 0; i < n; i++)
            s.push_back(g_http_answer_headers[i]);
    }
    return s;
}
