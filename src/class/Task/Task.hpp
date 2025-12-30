#ifndef TASK_HPP
#define TASK_HPP

// #include "webserv.hpp"

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
class Task {

public:
    std::string             _buffer; 
    http_request&           _request;
    http_answer&            _answer;
    const ServerSettings&  _settings;

    int             _status; // 404

    Task(Connection& connec, Server& s) : 
        _request(connec._request), _answer(connec._answer), _settings(s.getSettings()), _status(0) {}

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
    Ft_get(Connection& connec, Server& s) : Task(connec, s) {

        std::string server_path = _settings.find_setting("root");
        if (server_path.empty()) {
            std::cerr << ERR7 "ERROR?" << std::endl;
        }
        std::string requested_path = server_path + "/" + _request.path;
        if (access(requested_path.c_str(), F_OK)) {
            std::cerr << ERR7 "path invalid: " << requested_path << std::endl;
            printErr(RED "access" RESET);
            _status = 404;
            return;
            if (access(requested_path.c_str(), R_OK)) {
                std::cerr << ERR7 "not readable: " << requested_path << std::endl;
                printErr(RED "access" RESET);
                _status = 403;
                return;
            }
        }
        struct stat st;
        if (stat(requested_path.c_str(), &st) == 0)
            if (S_ISDIR(st.st_mode))
                requested_path += "/index.html";

        // ....
    }
    int ft_do() {
        std::cout << C_431 "IM ALIVE! (GET)" RESET << std::endl;
        return 0;
    }
    // ....
};



#endif