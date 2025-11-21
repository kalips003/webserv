#ifndef TASK_HPP
#define TASK_HPP

#include "webserv.hpp"

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
class Task {

    std::string     _buffer; 
    http_request&   _request;
    http_answer&    _answer;

public:
    int             _status; // 404

    Task(http_request& request, http_answer& answer) : 
        _request(request), _answer(answer), _status(0) {}

    virtual int ft_do() { return 0; }
};
///////////////////////////////////////////////////////////////////////////////]
/*
| Method      | Purpose / Client asks                         | Body allowed?                                |
| ----------- | --------------------------------------------- | -------------------------------------------- |
| **GET**     | Request a resource                            | No body (technically allowed but ignored)    |
| **HEAD**    | Request headers of resource                   | No body                                      |
| **POST**    | Submit data to server (form, JSON, file)      | Usually yes, body contains data              |
| **PUT**     | Replace resource at URL                       | Usually yes, body contains full new resource |
| **PATCH**   | Partial update of resource                    | Usually yes, body contains update            |
| **DELETE**  | Remove resource                               | Rarely has body; most servers ignore it      |
| **OPTIONS** | Ask server for allowed cmethods / capabilities | Can have body, but uncommon                  |
| **CONNECT** | Ask server to open a tunnel (HTTPS proxy)     | No body; used to establish tunnel            |
| **TRACE**   | Echo back the request                         | No body                                      |
*/



// once we know the method after header parsing, we can know how to 
//      execute it





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
class   Ft_get : public Task {



public:
    Ft_get(http_request& request, http_answer& answer) : 
        Task(request, answer) {}
    int ft_do() {
        std::cout << C_431 "IM ALIVE! (GET)" RESET << std::endl;
        return 0;
    }
    // ....
};



#endif