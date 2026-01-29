#ifndef HTTPA_HPP
#define HTTPA_HPP

#include "HttpObj.hpp"

///////////////////////////////////////////////////////////////////////////////]
// "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n<body>"
///////////////////////////////////////////////////////////////////////////////]
class HttpAnswer : public HttpObj {

private:
///////////////////////////////////////////////////////////////////////////////]
    std::string		_version; // HTTP/1.1
    int				_status_num; // 200
    std::string		_status_msg; // OK

	enum BodyMode	_body_type; // content-length mode; chunked-transfer mode; no body expected (GET, HEAD); multipart (POST form/file)
///////////////////////////////////////////////////////////////////////////////]

public:
	HttpAnswer() : HttpObj(), _version("HTTP/1.1"), _status_num(200), _status_msg("OK"), _body_type(BODY_NONE) { _status = SENDING_HEAD; }

//-----------------------------------------------------------------------------]
	/***  VIRTUALS  ***/
public:
	int		validateBodyWithoutLength() { return static_cast<int>(DOING); }
	int		isFirstLineValid(int fd);

///////////////////////////////////////////////////////////////////////////////]
public:
	void	createError(int errCode);
	void	setFirstLine(int errCode);
	void	initializationBeforeSend();


///////////////////////////////////////////////////////////////////////////////]
	/***  GETTERS  ***/
public:
	const std::string&	getVersion() { return _version; }
	int					getStatusNum() { return _status_num; }
	const std::string&	getStatusMsg() { return _status_msg; }
	temp_file&			getTempFile() { return _tmp_file; }
//-----------------------------------------------------------------------------]
private:

///////////////////////////////////////////////////////////////////////////////]
	/***  SETTERS  ***/
public:
//-----------------------------------------------------------------------------]
private:
///////////////////////////////////////////////////////////////////////////////]

	/***  FRIENDS  ***/
	friend std::ostream& operator<<(std::ostream& os, const HttpAnswer& r);
};

std::ostream& operator<<(std::ostream& os, const HttpAnswer& r);

#endif
