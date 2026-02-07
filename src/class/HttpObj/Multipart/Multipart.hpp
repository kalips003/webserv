#ifndef HTTPMULTIPART_HPP
#define HTTPMULTIPART_HPP

#include "HttpObj.hpp"

///////////////////////////////////////////////////////////////////////////////]
// "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n<body>"
///////////////////////////////////////////////////////////////////////////////]
class HttpMultipart : public HttpObj {

private:
///////////////////////////////////////////////////////////////////////////////]
	std::string		_delim; // boundary=----geckoformboundaryeb47a963
	std::string		_name; // name="foo" (mandatory)
	std::string		_filename; // filename="bar.txt" (optional)

///////////////////////////////////////////////////////////////////////////////]

public:
	HttpMultipart(const Settings::server_setting* settings, const std::string& delim) : 
		HttpObj(settings),
		_delim(delim) { _status = READING_FIRST; }

	HttpMultipart(const HttpMultipart& other);


	int			readingBody(char *buff, size_t sizeofbuff, int fd, ReadFunc reader);
	int			tool_check_next_two_char(int fd);
//-----------------------------------------------------------------------------]
	/***  VIRTUALS  ***/
public:
	virtual int		isFirstLineValid(int fd);
	virtual int		parseHeadersForValidity();
	virtual int		readBody(char *buff, size_t sizeofbuff, int fd, ReadFunc reader) { return this->readingBody(buff, sizeofbuff, fd, reader); }

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
	void				setFileNName(const std::string& s) { _filename = s; }
//-----------------------------------------------------------------------------]
private:
///////////////////////////////////////////////////////////////////////////////]

	/***  FRIENDS  ***/
	// friend std::ostream& operator<<(std::ostream& os, const HttpMultipart& r);
};

// std::ostream& operator<<(std::ostream& os, const HttpMultipart& r);

#endif
