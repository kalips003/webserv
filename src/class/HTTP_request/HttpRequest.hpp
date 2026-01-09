#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <sys/types.h>

#include <defines.hpp>

///////////////////////////////////////////////////////////////////////////////]
// "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n<body>"
///////////////////////////////////////////////////////////////////////////////]
/**
 * Represents a Request HTTP
 *
 * Owns all the parsings tools
 */
class HttpRequest {

private:
///////////////////////////////////////////////////////////////////////////////]
    std::string         _method; // GET
    std::string         _path; // /index.html
    std::string         _version; // HTTP/1.1

    map_strstr          _headers; // Accept: html

    std::string         _body; // buffer for the body
    std::string         _buffer; // buffer

    ssize_t             _body_size; // size of body in bytes, -1 if no body?
    size_t              _body_bytes_received;

    int                 _fd_body; // if body too large, use a temp file, -1 if no body?
    std::string         _tmp_body_path; // "./tmp/body_12345.tmp"
    enum BodyMode       _body_type; // content-length mode; chunked-transfer mode; no body expected (GET, HEAD); multipart (POST form/file)

	bool				_bodyistemp; // if no need to keep the temp body file, delete from temp
///////////////////////////////////////////////////////////////////////////////]

public:
    HttpRequest() : _body_size(0), _body_bytes_received(0), _fd_body(-1),
        _body_type(BODY_NONE) {}
    ~HttpRequest();

//-----------------------------------------------------------------------------]
public:
    int    	readingFirstLine(std::string& str_buff);
	
private:
    bool	isPathValid(std::string& path);

//-----------------------------------------------------------------------------]
public:
    int		readingHeaders(std::string& buff);

private:
	int		parsingHeaders(std::string& delim);
	int 	parse_header_for_syntax();
	int		parse_headers_for_validity();

public:
//-----------------------------------------------------------------------------]
	int		readingBody(std::string& buff);


public:
//-----------------------------------------------------------------------------]
	int		openFdBody(const char* path);
	
///////////////////////////////////////////////////////////////////////////////]
/***  GETTERS  ***/
    const std::string& getMethod() const { return _method; }
    const std::string& getPath() const { return _path; }
    const std::string& getVersion() const { return _version; }
    const map_strstr& getHeaders() const { return _headers; }
// 
    const std::string& getBody() const { return _body; }
    const std::string& getBuffer() const { return _buffer; }
// 
    int 	getFdBody() const { return _fd_body; }
    size_t 	getBodyBytesReceived() const { return _body_bytes_received; }
    size_t 	getBodySize() const { return _body_size; }
//-----------------------------------------------------------------------------]
    ssize_t      	isThereBody() const ;
    std::string 	find_setting(const std::string& set) const ;

/***  SETTERS  ***/
    void   addBuffer(std::string& s) { _buffer += s; };
///////////////////////////////////////////////////////////////////////////////]

};

std::ostream& operator<<(std::ostream& os, HttpRequest& r);

#endif
