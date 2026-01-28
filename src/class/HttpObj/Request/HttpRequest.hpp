#ifndef HTTPR_HPP
#define HTTPR_HPP

#include "HttpObj.hpp"

///////////////////////////////////////////////////////////////////////////////]
// "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n<body>"
///////////////////////////////////////////////////////////////////////////////]
class HttpRequest : public HttpObj {

private:
///////////////////////////////////////////////////////////////////////////////]
	std::string		_method; // GET
	std::string		_path; // /index.html
	std::string		_version; // HTTP/1.1

	enum BodyMode	_body_type; // content-length mode; chunked-transfer mode; no body expected (GET, HEAD); multipart (POST form/file)
///////////////////////////////////////////////////////////////////////////////]

public:
	HttpRequest() : HttpObj(), _body_type(BODY_NONE) { _status = READING_FIRST; }

//-----------------------------------------------------------------------------]
	/***  VIRTUALS  ***/
public:
	int		validateBodyWithoutLength();
	int		isFirstLineValid(int fd);

///////////////////////////////////////////////////////////////////////////////]
public:


///////////////////////////////////////////////////////////////////////////////]
/***  GETTERS  ***/
public:
	const std::string&	getMethod() const { return _method; }
	const std::string&	getPath() const { return _path; }
	const std::string&	getVersion() const { return _version; }
	temp_file&			getTempFile() { return _tmp_file; }
//-----------------------------------------------------------------------------]
private:

/***  SETTERS  ***/
public:
//-----------------------------------------------------------------------------]
private:
///////////////////////////////////////////////////////////////////////////////]

	/***  FRIENDS  ***/
	friend std::ostream& operator<<(std::ostream& os, const HttpRequest& r);
};

std::ostream& operator<<(std::ostream& os, const HttpRequest& r);

#endif
