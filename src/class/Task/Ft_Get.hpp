#ifndef FT_GET_HPP
#define FT_GET_HPP

#include "Task.hpp"

///////////////////////////////////////////////////////////////////////////////]
//* GET *     | Request a resource  | No body (technically allowed but ignored)|
///////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
/*
| Header              | Purpose                                                                   |
| ------------------- | ------------------------------------------------------------------------- |
| `Host`              | mandatory in HTTP/1.1, specifies which host the request targets           |
| `User-Agent`        | client info (browser, curl, etc.)                                         |
| `Accept`            | media types the client can handle (`text/html`, `application/json`, etc.) |
| `Accept-Encoding`   | compression formats client supports (`gzip`, `deflate`, `br`)             |
| `Accept-Language`   | preferred languages                                                       |
| `Connection`        | e.g., `keep-alive`, `close`                                               |
| `Cache-Control`     | caching directives, e.g., `no-cache`, `max-age=0`                         |
| `If-Modified-Since` | conditional GET: only send if resource modified since given date          |
| `If-None-Match`     | conditional GET using ETag                                                |
| `Referer`           | URL of the page linking to this resource                                  |
| `Cookie`            | session cookies, auth tokens                                              |
*/

// GET /index.html HTTP/1.1
// Host: example.com
// User-Agent: curl/8.0
// Accept: text/html
// Accept-Encoding: gzip, deflate
// Connection: keep-alive
class   Ft_Get : public Task {


public:
    Ft_Get(Connection& connec, Server& s) : Task(connec, s) {}

    //     std::string server_path = _settings.find_setting("root");
    //     if (server_path.empty()) {
    //         std::cerr << ERR7 "ERROR?" << std::endl;
    //     }
    //     std::string requested_path = server_path + "/" + _request.path;
    //     if (access(requested_path.c_str(), F_OK)) {
    //         std::cerr << ERR7 "path invalid: " << requested_path << std::endl;
    //         printErr(RED "access" RESET);
    //         _status = 404;
    //         return;
    //         if (access(requested_path.c_str(), R_OK)) {
    //             std::cerr << ERR7 "not readable: " << requested_path << std::endl;
    //             printErr(RED "access" RESET);
    //             _status = 403;
    //             return;
    //         }
    //     }
    //     struct stat st;
    //     if (stat(requested_path.c_str(), &st) == 0)
    //         if (S_ISDIR(st.st_mode))
    //             requested_path += "/index.html";

    //     // ....
    // }
    // int ft_do() {
    //     std::cout << C_431 "IM ALIVE! (GET)" RESET << std::endl;
    //     return 0;
    // }
    // ....
};



#endif