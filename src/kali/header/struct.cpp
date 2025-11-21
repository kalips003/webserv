#include "webserv.hpp"
#include "structs.hpp"
#include "defines.hpp"

///////////////////////////////////////////////////////////////////////////////]
// http_request
///////////////////////////////////////////////////////////////////////////////]
bool    atoi_v2(const std::string& input, int& rtrn);
ssize_t      http_request::isThereBody() {

    std::map<std::string, std::string>::iterator it = headers.find("body-size");
    if (it == headers.end())
        return 0;
    int r;
    if (!atoi_v2(it->second, r) || r < 0)
        return -1;
    return static_cast<ssize_t>(r);
}
// struct http_request {

//     std::string method; // GET
//     std::string path; // /index.html
//     std::string version; // HTTP/1.1

//     std::map<std::string, std::string>  headers; 

//     std::string body;
//     int         fd_body;
//     size_t      body_bytes_received;
//     ssize_t     body_size;
//     enum BodyMode   body_type; // content-length mode; chunked-transfer mode; no body expected (GET, HEAD); multipart (POST form/file)

//     int         header_delim_progress; 
std::ostream& operator<<(std::ostream& os, http_request& r) {

    os << C_542 "---------------------------------------------\n" RESET;
    os << C_542 "\tREQUEST:\n" RESET;
    os << r.method << " " << r.path << " " << r.version << std::endl;
    os << C_542 "\tHEADERS:\n" RESET;
    for (map_strstr::iterator  h = r.headers.begin(); h != r.headers.end(); h++) {
        os << C_114 << h->first << RESET ": " << h->second << std::endl;
    }

    os << C_542 "\tBODY:\n[" RESET << r.body << C_542 "]" RESET << std::endl;
    os << C_542 "fd_body: " RESET << r.fd_body << std::endl;
    os << C_542 "body_bytes_received: " RESET << r.body_bytes_received << std::endl;
    os << C_542 "body_size: " RESET << r.body_size << std::endl;         
    os << C_542 "---------------------------------------------\n" RESET;
    return os;       
}

///////////////////////////////////////////////////////////////////////////////]
// http_answer
///////////////////////////////////////////////////////////////////////////////]
static std::string  concatenateHeaders(map_strstr& headers);
std::string itostr(int n);
///////////////////////////////////////////////////////////////////////////////]
// use the filled _answer to fill body, set bytes_tosend ...
void http_answer::http_answer_ini() {

    std::string     headers;
    headers.reserve(4096); // optional
    headers = version + " " + itostr(status) + " " + msg_status 
        + "\r\n" + concatenateHeaders(_headers) + "\r\n";

    head.reserve(headers.size() + head.size());
    head.insert(0, headers);
    _full_size += head.size();
}

/*  return a string of all headers separated by '\r\n' */
static std::string    concatenateHeaders(map_strstr& headers) {
    std::string s;
    for (map_strstr::const_iterator it = headers.begin(); it != headers.end(); it++)
        s += it->first + ": " + it->second + "\r\n";

    return s;
}