#ifndef HTTPOBJ_HPP
#define HTTPOBJ_HPP

#include <string>
#include <map>
#include <sys/types.h>

#include "_colors.h"
#include "defines.hpp"


///////////////////////////////////////////////////////////////////////////////]
/*
	start-line
	headers
	CRLF
	body
*/
///////////////////////////////////////////////////////////////////////////////]
class httpObj {

protected:
///////////////////////////////////////////////////////////////////////////////]
	std::string		_start;

	map_strstr		_headers;

	std::string		_body;

	int				_fd_body;
	std::string		_tmp_body_path; // "./tmp/body_12345.tmp"
	size_t			_body_size;

	std::string		_buffer;
///////////////////////////////////////////////////////////////////////////////]

public:
	httpObj() : _fd_body(-1), _body_size(0) {}
	~httpObj();
		

///////////////////////////////////////////////////////////////////////////////]
/***  GETTERS  ***/
public:
    const map_strstr&  getHeaders() const { return _headers; }
    int                getBodyFd() const { return _fd_body; }
    size_t             getFullSize() const { return _body_size; }
//-----------------------------------------------------------------------------]
    std::string 	   find_setting(const std::string& set) const ;
    AnswerStatus	   isThereBody() const ;

/***  SETTERS  ***/
public:
	void	setStrBody(std::string& s) { _body = s; _body_size = s.size(); }
	void	setFd(int fd) { _fd_body = fd; }
	void	setBodySize(size_t size);
	void	setFirstLine(int code);
//-----------------------------------------------------------------------------]
    void    addToHeaders(const std::string& parameter, const std::string& value) { _headers[parameter] = value; }
///////////////////////////////////////////////////////////////////////////////]

	/***  FRIENDS  ***/
	friend std::ostream& operator<<(std::ostream& os, httpObj& r);

};

std::ostream& operator<<(std::ostream& os, httpObj& r);

#endif
