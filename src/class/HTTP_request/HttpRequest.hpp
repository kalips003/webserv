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
    std::string         _buffer; // buffer ???

    int                 _fd_body; // if body too large, use a temp file, -1 if no body?
    ssize_t             _body_size; // size of body in bytes, -1 if no body?
    size_t              _body_bytes_received;
    enum BodyMode       _body_type; // content-length mode; chunked-transfer mode; no body expected (GET, HEAD); multipart (POST form/file)

    int                 _header_delim_progress; // where are we of "\r\n\r\n"
///////////////////////////////////////////////////////////////////////////////]

public:
    HttpRequest() : _fd_body(-1), _body_size(0), _body_bytes_received(0),
        _body_type(BODY_NONE), _header_delim_progress(0) {}
    ~HttpRequest();

//-----------------------------------------------------------------------------]
/** parse the headers to find if there is a body, return its length
*
* @return length of "body-size"
*
* if no "body-size" is found, return 0,
* if "body-size" is incorrect, return -1			---*/
    ssize_t      	isThereBody() const ;

//-----------------------------------------------------------------------------]
/** find the given setting in the _headers
*	@return the string value of the setting
*
* if setting not found, return "" empty string	---*/
    std::string 	find_setting(const std::string& set) const ;

//-----------------------------------------------------------------------------]
/** Add given string to _buffer, parse it for "\r\n"
* 
* if found, parse the HEAD of request, and remove it from _buffer
* 
* @return if delim found, return READING_HEADER if HEAD valid, errCode if not
* 
* if delim ("\r\n") not found, return FIRST			---*/
    int    	parse_header_first_read(std::string& str_buff);
	
//-----------------------------------------------------------------------------]
/** Check the syntax validity of the path, doesnt check if file exist	---*/
    bool	isPathValid(std::string& path);

//-----------------------------------------------------------------------------]
/** check _buffer + buff for \r\n\r\n
*
* @return READING_HEADER if not found
*
* DOING if no body
*
* READING_BODY if body and parsing went well
* 
* errCode if parsing went bad			---*/
    int    check_buffer_for_rnrn(std::string& buff);

///////////////////////////////////////////////////////////////////////////////]
/*  GETTERS  */
    const std::string& getMethod() const { return _method; }
    const std::string& getPath() const { return _path; }
    const std::string& getVersion() const { return _version; }
    const map_strstr& getHeaders() const { return _headers; }
// 
    const std::string& getBody() const { return _body; }
    const std::string& getBuffer() const { return _buffer; }
// 
    int getFdBody() const { return _fd_body; }
    size_t getBodyBytesReceived() const { return _body_bytes_received; }
    size_t getBodySize() const { return _body_size; }
/*  SETTERS  */
    void   addBuffer(std::string& s) { _buffer += s; };
///////////////////////////////////////////////////////////////////////////////]
    void    f1(std::string& str_buff);
	// bool HttpRequest::parse_header_for_syntax();

};

std::ostream& operator<<(std::ostream& os, HttpRequest& r);

#endif