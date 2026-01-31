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
	std::string		_name; // name="foo"
	std::string		_filename; // filename="bar.txt"

///////////////////////////////////////////////////////////////////////////////]

public:
	HttpMultipart(const std::string& s, const std::string& leftovers) : 
		HttpObj(),
		_delim(s) { _status = READING_FIRST; 
					_leftovers = leftovers; }

	HttpMultipart(const HttpMultipart& other);

int		parse_multifile(char *buff, size_t sizeofbuff, int fd);
int		readingBody(char *buff, size_t sizeofbuff, int fd);

//-----------------------------------------------------------------------------]
	/***  VIRTUALS  ***/
public:
	virtual int		isFirstLineValid(int fd);
	virtual int		parseHeadersForValidity();

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
