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


class Sink;
///////////////////////////////////////////////////////////////////////////////]
typedef ssize_t (*ReadFunc)(int, void*, size_t);
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
	static ssize_t	readBuffer(char *buff, size_t sizeofbuff, int fd, std::string& to_append_to, ReadFunc reader);
	ssize_t			readForDelim(char *buff, size_t sizeofbuff, int fd, const std::string& delim, int remove_delim, Sink& to_store_to, ReadFunc reader);
public:
	int				receive(char *buff, size_t sizeofbuff, int fd, ReadFunc reader);
		int			readingFirstLine(char *buff, size_t sizeofbuff, int fd, ReadFunc reader);
		int			readingHeaders(char *buff, size_t sizeofbuff, int fd, ReadFunc reader);
		int			streamingBody(char *buff, size_t sizeofbuff, int fd, ReadFunc reader);
	int				receive_cgi(char *buff, size_t sizeofbuff, int fd);


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
	virtual int		isFirstLineValid(int fd) { (void)fd; return 0; }
	virtual int		parseHeadersForValidity() { return 0; }
	virtual int		readBody(char *buff, size_t sizeofbuff, int fd, ReadFunc reader) { return streamingBody(buff, sizeofbuff, fd, reader); }

//-----------------------------------------------------------------------------]
	/***  TOOLS  ***/
public:
	void	concatenateIntoHead();
	int		parse_head_for_headers();
	int		findDelimInLeftovers(const std::string& delim, int remove_delim, Sink& to_append_to);


///////////////////////////////////////////////////////////////////////////////]
	/***  GETTERS  ***/
public:
	const map_strstr&	getHeaders() const { return _headers; }
	int					getBodyFd() const { return _tmp_file._fd; }
	HttpBodyStatus		getStatus() const { return _status; }
	ssize_t				getFullSize() { return _tmp_file.getBodySize(); }
	temp_file&			getFile() { return _tmp_file; }
	std::string&		getBuffer() { return _buffer; }
	std::string&		getLeftovers() { return _leftovers; }
	std::string&		getFirst() { return _first; }
//-----------------------------------------------------------------------------]
public:
	const std::string*	find_in_headers(const std::string& set) const ;
	std::string*		find_in_headers(const std::string& set);
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
	void				setBytesTotal(ssize_t b) { _bytes_total = b; }
	void				setBytesWritten(size_t b) { _bytes_written = b; }
	void				setStatus(HttpBodyStatus s) { _status = s; }
//-----------------------------------------------------------------------------]
private:
///////////////////////////////////////////////////////////////////////////////]

	/***  FRIENDS  ***/
	friend std::ostream& operator<<(std::ostream& os, const HttpObj& r);

};

std::ostream& operator<<(std::ostream& os, const HttpObj& r);

///////////////////////////////////////////////////////////////////////////////]
/**	Pure virtual to write either to a string& or append to a fd 		   ---*/
class Sink {
public:
	virtual bool write(const char* data, size_t len) = 0;
	virtual ~Sink() {}  // always add virtual destructor
};

class StringSink : public Sink {
	std::string& s;
public:
	StringSink(std::string& ref) : s(ref) {}
	bool write(const char* data, size_t len);
};

class FileSink : public Sink {
	temp_file& f;
public:
	FileSink(temp_file& tf) : f(tf) {}
	bool write(const char* data, size_t len);
};
///////////////////////////////////////////////////////////////////////////////]

ssize_t recv0(int fd, void* buf, size_t n);

#endif
