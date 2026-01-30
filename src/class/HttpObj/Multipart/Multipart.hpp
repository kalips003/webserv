#ifndef HTTPMULTIPART_HPP
#define HTTPMULTIPART_HPP

#include "HttpObj.hpp"

///////////////////////////////////////////////////////////////////////////////]
// "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n<body>"
///////////////////////////////////////////////////////////////////////////////]
class HttpMultipart : public HttpObj {

private:
///////////////////////////////////////////////////////////////////////////////]
	std::string		_delim;
	std::string		_name; // GET
	std::string		_filename; // /index.html

	enum BodyMode	_body_type; // content-length mode; chunked-transfer mode; no body expected (GET, HEAD); multipart (POST form/file)
///////////////////////////////////////////////////////////////////////////////]

public:
	HttpMultipart(const std::string& s) : 
		HttpObj(),
		_delim(s),
		_body_type(BODY_NONE) { _status = READING_FIRST; }


//-----------------------------------------------------------------------------]
	/***  VIRTUALS  ***/
public:
	virtual int		isFirstLineValid(int fd) { (void)fd; return 0; }
	virtual int		parseHeadersForValidity() { return 0; }

///////////////////////////////////////////////////////////////////////////////]
public:

///////////////////////////////////////////////////////////////////////////////]
/***  GETTERS  ***/
public:
	const std::string&	getDelim() const { return _delim; }
	const std::string&	getName() const { return _name; }
	const std::string&	getFilename() const { return _filename; }
//-----------------------------------------------------------------------------]
private:
/***  SETTERS  ***/
public:
//-----------------------------------------------------------------------------]
private:
///////////////////////////////////////////////////////////////////////////////]

	/***  FRIENDS  ***/
	friend std::ostream& operator<<(std::ostream& os, const HttpMultipart& r);
};

std::ostream& operator<<(std::ostream& os, const HttpMultipart& r);

#endif
