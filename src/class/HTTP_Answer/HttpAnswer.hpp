#ifndef HTTPANSWER_HPP
#define HTTPANSWER_HPP

#include <string>
#include <map>

#include "_colors.h"
#include "defines.hpp"

///////////////////////////////////////////////////////////////////////////////]
/*
    <Status Line> = HTTP/1.1 200 OK (\r\n)
    <Headers>                       (\r\n)
                                    (empty line separating) (\r\n\r\n)
    <Body (optional)>
*/
///////////////////////////////////////////////////////////////////////////////]
// "HTTP/1.1 200 OK\r\n\r\n"
class httpAnswer {

public:
    std::string     _version; // HTTP/1.1
    int             _status; // 200
    std::string     _msg_status; // OK

    map_strstr      _headers; // Content-Length: 25

    std::string     _head; // 1) <body>, after ini(): head<body>
    std::string     _body_leftover;
    int             _fd_body;

    size_t          _full_size;
    size_t          _bytes_sent;


    httpAnswer() : _version("HTTP/1.1"), _status(200), _msg_status("OK"), 
        _fd_body(-1), _full_size(0), _bytes_sent(0) {}
    ~httpAnswer();
    
/*  take the filled answer, concatenate headers into _head */
    void http_answer_ini();
/*  concatenate and return first line: <HTTP/1.1 200 OK\r\n\r\n> */
    std::string rtrnFistLine();
/*  send the next chunk of data into the buffer provided, to client fd */
    enum ConnectionStatus   sendToBuffer(int fd, char* buff, size_t sizeofbuff);
    bool    addToHeaders(std::string& parameter, std::string& value);
};

std::ostream& operator<<(std::ostream& os, httpAnswer& r);

///////////////////////////////////////////////////////////////////////////////]
/*
HTTP Response Headers

Accept-Ranges:                 Indicates if server supports range requests (bytes)
Age:                           Age of the cached response in seconds
Allow:                         Methods allowed for the resource (GET, POST, OPTIONS)
Cache-Control:                 Caching rules (no-store, private, max-age)
Connection:                     keep-alive or close
Content-Encoding:               Compression format applied (gzip, br)
Content-Language:               Language of the content (en-US)
Content-Length:                 Length of the response body in bytes
Content-Location:               Alternate location for the returned content
Content-Disposition:            How content should be handled (inline, attachment; filename="file.txt")
Content-Type:                   MIME type of response (text/html, application/json)
Date:                           Date/time of response
ETag:                           Unique identifier for version of resource
Expires:                        Expiration date for caching
Last-Modified:                  Last modification date of resource
Location:                       Redirect target URL (3xx responses)
Pragma:                         Legacy caching instructions (no-cache)
Retry-After:                    When to retry (503 maintenance)
Server:                         Server software/version (nginx/1.25)
Set-Cookie:                     Cookie sent to client
Strict-Transport-Security:      HSTS rules (HTTPS only)
Trailer:                        Headers sent after chunked body
Transfer-Encoding:              Encoding applied (chunked)
Upgrade:                        Protocol upgrades (websocket)
Vary:                           Specifies headers that affect caching (Accept-Encoding)
Via:                            Proxy chain
Warning:                        General warnings
WWW-Authenticate:               Challenge for authentication (Basic, Digest)
*/
///////////////////////////////////////////////////////////////////////////////]





#endif