#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>

#include <netinet/in.h>

#include "defines.hpp"

#include "HttpAnswer.hpp"
#include "HttpRequest.hpp"

class Task;
class ServerSettings;
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
class Connection {

private:
///////////////////////////////////////////////////////////////////////////////]
    int                 _client_fd; // fd associated with this client connection
    struct sockaddr_in  _client_addr; // struct with informations about the client
    socklen_t           _addr_len; // ?

    HttpRequest         _request;
    httpAnswer          _answer;
    Task                *_body_task;

    ConnectionStatus    _status; // FIRST = 0, READING_HEADER, READING_BODY, DOING, SENDING, CLOSED
///////////////////////////////////////////////////////////////////////////////]

public:
	Connection() :
		_client_fd(-1), _addr_len(sizeof(_client_addr)), 
		_body_task(NULL), _status(FIRST) {}

	Connection(int fd, struct sockaddr_in c, socklen_t al) :
		_client_fd(fd), _client_addr(c), _addr_len(al), 
		_body_task(NULL), _status(FIRST) {}
	
	~Connection();

//-----------------------------------------------------------------------------]
public:
	bool					ft_update(char *buff, size_t sizeofbuff);
//-----------------------------------------------------------------------------]
public:
    enum ConnectionStatus	ft_read(char *buff, size_t sizeofbuff);

//-----------------------------------------------------------------------------]
public:
    enum ConnectionStatus	ft_doing( void );

//-----------------------------------------------------------------------------]
public:
    enum ConnectionStatus	ft_send(char *buff, size_t sizeofbuff);



//-----------------------------------------------------------------------------]
public:
///////////////////////////////////////////////////////////////////////////////]
/***  GETTERS  ***/
    int                         getClientFd() const { return _client_fd; }
    const struct sockaddr_in&   getClientAddr() const { return _client_addr; }
    socklen_t                   getAddrLen() const { return _addr_len; }
//
    HttpRequest&                getRequest() { return _request; }
    httpAnswer&                 getAnswer() { return _answer; }
//
    Task*                       getBodyTask() const { return _body_task; }
    ConnectionStatus            getStatus() const { return _status; }
//
	const std::string			findRequestHeader(std::string header) { return _request.find_setting(header); }
	// const std::string&			findAnswerHeader(std::string header) { return _answer.find_setting(header); }

/***  SETTERS  ***/
	void	setStatus(ConnectionStatus s) { _status = s; }
//
	void	resetRequest( void ) { _request = HttpRequest(); }
	void	resetAnswer( void ) { _answer = httpAnswer(); }
	void	closeFd();
///////////////////////////////////////////////////////////////////////////////]

};

std::ostream& operator<<(std::ostream& os, const Connection& c);

#endif