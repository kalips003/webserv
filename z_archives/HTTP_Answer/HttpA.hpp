#ifndef HTTPANSWER_HPP
#define HTTPANSWER_HPP

#include <string>
#include <map>
#include <sys/types.h>

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
class httpa {

private:
///////////////////////////////////////////////////////////////////////////////]
    std::string     _version; // HTTP/1.1
    int             _status; // 200
    std::string     _msg_status; // OK

    map_strstr      _headers; // Content-Length: 25

    std::string     _head; // "HTTP/1.1 200 OK\r\n<headers>\r\n\r\n"
    std::string     _body;
    std::string     _leftover; // leftover of a partial send of data from fd_file
    int             _fd_body;
					
    size_t          _body_size;
    size_t          _bytes_sent;
	AnswerStatus	_sending_status; // SENDING_HEAD = 0, SENDING_BODY, SENDING_BODY_FD, ENDED
///////////////////////////////////////////////////////////////////////////////]

public:
    httpa() : _version("HTTP/1.1"), _status(200), _msg_status("OK"), 
        _fd_body(-1), _body_size(0), _bytes_sent(0), _sending_status(SENDING_HEAD) {}
    ~httpa();
    
//-----------------------------------------------------------------------------]
public:
	enum ConnectionStatus    create_error(int errCode);

//-----------------------------------------------------------------------------]
public:
	void			http_answer_ini();
	std::string		rtrnFirstLine();
private:
	std::string		concatenateHeaders();

//-----------------------------------------------------------------------------]
public:
	enum ConnectionStatus	sending(char *buff, size_t size_buff, int fd_client);
private:
	ssize_t					fillBuffer(char *buff, size_t size_buff);
	void					updateAfterSend(char *buff, ssize_t bytesLoaded, ssize_t bytesSent);


//-----------------------------------------------------------------------------]
	
public:
	void					defaultHeaders();
//-----------------------------------------------------------------------------] 
///////////////////////////////////////////////////////////////////////////////]
/***  GETTERS  ***/
    const std::string& getVersion() const { return _version; }
    int                getStatus()  const { return _status; }
    const std::string& getMsgStatus() const { return _msg_status; }
//
    const map_strstr&  getHeaders() const { return _headers; }
//
    const std::string& getHead() const { return _head; }
    const std::string& getBodyLeftover() const { return _leftover; }
    int                getBodyFd() const { return _fd_body; }
//
    size_t             getFullSize() const { return _body_size; }
    size_t             getBytesSent() const { return _bytes_sent; }
//-----------------------------------------------------------------------------]
    std::string 	   find_setting(const std::string& set) const ;
    AnswerStatus	   isThereBody() const ;

/***  SETTERS  ***/

	void	setStrBody(std::string& s) { _body = s; _body_size = s.size(); }
	void	setVersion(std::string& s) { _version = s; }
	void	setFd(int fd) { _fd_body = fd; }
	void	setBodySize(size_t size);
	void	setStatus(int errCode) { _status = errCode; }
	void	setMsgStatus(std::string& s) { _msg_status = s; }
	void	setFirstLine(int code);
//-----------------------------------------------------------------------------]
    void    addToHeaders(const std::string& parameter, const std::string& value) { _headers[parameter] = value; }
///////////////////////////////////////////////////////////////////////////////]
};

std::ostream& operator<<(std::ostream& os, httpa& r);

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