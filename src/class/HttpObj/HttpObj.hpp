#ifndef HTTPOBJ_HPP
#define HTTPOBJ_HPP

#include "Log.hpp"
#include <string>
#include <map>
#include <sys/types.h>

#include "_colors.h"
#include "defines.hpp"
#include <sys/stat.h>

#include "TempFile.hpp"
///////////////////////////////////////////////////////////////////////////////]
/*
	start-line
	headers
	CRLF
	body
*/
///////////////////////////////////////////////////////////////////////////////]
class HttpObj {

public:
	enum HttpBodyStatus {
	
		CLOSED = 0,
		READING_FIRST,
		READING_HEADER,
		READING_BODY,
		DOING,
		SENDING_HEAD,
		SENDING_BODY,
		SENDING_BODY_FD
	};

protected:
///////////////////////////////////////////////////////////////////////////////]
	std::string		_first; // First line of the exchange
	std::string		_head; // First line + headers

	map_strstr		_headers;

	std::string		_body; // Small string body

	temp_file		_tmp_file; // File associated with the body

	std::string		_buffer;
	std::string		_leftovers;

	ssize_t			_bytes_total; // size of body in bytes (0 if no body)
	size_t			_bytes_written; // size in bytes already sent / received

	HttpBodyStatus	_status;
///////////////////////////////////////////////////////////////////////////////]

public:
	HttpObj() : _tmp_file(), _bytes_total(0), _bytes_written(0), _status(CLOSED) {}
	~HttpObj() {}


//-----------------------------------------------------------------------------]
	/***  READ  ***/
public:
	static ssize_t	readBuffer(char *buff, size_t sizeofbuff, int fd, std::string& to_append_to);
	ssize_t 		read_until_delim_is_found(char *buff, size_t sizeofbuff, int fd, const std::string& delimitor, bool& is_found);
public:
	int				receive(char *buff, size_t sizeofbuff, int fd);
		int			readingFirstLine(char *buff, size_t sizeofbuff, int fd);
		int			readingHeaders(char *buff, size_t sizeofbuff, int fd);
			int		parseHeaders();
			int		parse_buffer_for_headers();
		int			readingBody(char *buff, size_t sizeofbuff, int fd);


//-----------------------------------------------------------------------------]
	/***  SEND  ***/
public:
	static ssize_t	sendBufferString(char *buff, size_t sizeofbuff, int fd, std::string& to_send_from);
	ssize_t			sendBufferFile(char *buff, size_t sizeofbuff, int fd, int fd_file);
public:
	HttpBodyStatus	send(char *buff, size_t sizeofbuff, int fd);
private:
	HttpBodyStatus	whatToSend() const;

//-----------------------------------------------------------------------------]
	/***  VIRTUALS  ***/
public:
	virtual int		validateBodyWithoutLength() { return static_cast<int>(DOING); }
	virtual int		isFirstLineValid(int fd);

//-----------------------------------------------------------------------------]
	/***  TOOLS  ***/
public:
	void			concatenateIntoHead();


///////////////////////////////////////////////////////////////////////////////]
	/***  GETTERS  ***/
public:
	const map_strstr&	getHeaders() const { return _headers; }
	int					getBodyFd() const { return _tmp_file._fd; }
	HttpBodyStatus		getStatus() const { return _status; }
	ssize_t				getFullSize() { return _tmp_file.getBodySize(); }
	temp_file&			getFile() { return _tmp_file; }
//-----------------------------------------------------------------------------]
public:
	const std::string*	find_setting(const std::string& set) const ;
	ssize_t				isThereBodyinHeaders() const ;
	size_t				isThereBody();
///////////////////////////////////////////////////////////////////////////////]
	/***  SETTERS  ***/
public:
	void				setDefaultHeaders();
	void				addToHeaders(const std::string& parameter, const std::string& value) { _headers[parameter] = value; }
	void				setStringBody(const std::string& s) { _body = s; }
	void				setMIMEtype(const std::string& path);
	void				closeTemp(bool del);
//-----------------------------------------------------------------------------]
private:
///////////////////////////////////////////////////////////////////////////////]

	/***  FRIENDS  ***/
	friend std::ostream& operator<<(std::ostream& os, const HttpObj& r);

};

std::ostream& operator<<(std::ostream& os, const HttpObj& r);

#endif
