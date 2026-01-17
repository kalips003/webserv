#ifndef FT_GET_HPP
#define FT_GET_HPP

#include "Task.hpp"

#include <fstream>
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

private:


public:
    Ft_Get(Connection& connec) : Task(connec) {}

    int ft_do();

private:
    int serveAutoIndexing(const std::string& path);
	int serveFile(const std::string& path, struct stat& ressource_info);
    // ....
	int handleCGI(const std::string& ressource, const std::string& query, const std::string* CGI_interpreter_path);
};



#endif