#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "Log.hpp"
#include <string>
#include <netinet/in.h>

#include "defines.hpp"

#include "HttpAnswer.hpp"
#include "HttpRequest.hpp"

// #include "Method.hpp"

class Method;
class SettingsServer;
class Connection;

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
class Connection {

public:
	struct transfer_data {
		int			_client_fd; // fd associated with this client connection
		int			_epoll_fd;
		Connection*	_this_ptr; // ptr to this client connection
		char*		_buffer; // Shared Server buffer
		size_t		_sizeofbuff;

		transfer_data() : 
			_client_fd(-1), 
			_epoll_fd(-1), 
			_this_ptr(NULL),
			_buffer(NULL),
			_sizeofbuff(0) {}

		transfer_data(int fd, int e, Connection* c, char* buffer, size_t size) : 
			_client_fd(fd), 
			_epoll_fd(e), 
			_this_ptr(c), 
			_buffer(buffer),
			_sizeofbuff(size) {}

		friend std::ostream& operator<<(std::ostream& os, const transfer_data& t);
	};

	enum ConnectionStatus {
	
		READING,
		DOING,
		DOING_CGI,
		SENDING,
		CLOSED
	};

///////////////////////////////////////////////////////////////////////////////]
private:
	struct sockaddr_in  	_client_addr; // struct with informations about the client
	socklen_t				_addr_len; // ?

	HttpRequest				_request;
	HttpAnswer				_answer;

	Method					*_body_task;

	ConnectionStatus		_status;
	transfer_data			_data;
///////////////////////////////////////////////////////////////////////////////]

public:
	Connection() :
		_client_addr(), 
		_addr_len(sizeof(_client_addr)),
		_body_task(NULL), 
		_status(READING), 
		_data() { _data._this_ptr = this; }

	Connection(char* buffer, size_t size) :
		_client_addr(), 
		_addr_len(sizeof(_client_addr)),
		_body_task(NULL), 
		_status(READING), 
		_data() { _data._this_ptr = this; 
					_data._buffer = buffer; _data._sizeofbuff = size; }

	Connection(int fd, int epoll, struct sockaddr_in c, socklen_t al, char* buffer, size_t size) :
		_client_addr(c), 
		_addr_len(al), 
		_body_task(NULL), 
		_status(READING), 
		_data(fd, epoll, this, buffer, size) {}

	Connection(const Connection& other) :
		_client_addr(other._client_addr), 
		_addr_len(other._addr_len), 
		_body_task(NULL), 
		_status(READING), 
		_data(other._data) { _data._this_ptr = this; }

	~Connection();


//-----------------------------------------------------------------------------]
public:
	bool					ft_update(char *buff, size_t sizeofbuff);
//-----------------------------------------------------------------------------]

	ConnectionStatus		ft_read(char *buff, size_t sizeofbuff);
//-----------------------------------------------------------------------------]

	enum ConnectionStatus	ft_doing( void );
//-----------------------------------------------------------------------------]

	enum ConnectionStatus	ft_send(char *buff, size_t sizeofbuff);


///////////////////////////////////////////////////////////////////////////////]
	/***  GETTERS  ***/
public:
	int						 	getClientFd() const { return _data._client_fd; }
	const struct sockaddr_in&	getClientAddr() const { return _client_addr; }
	socklen_t					getAddrLen() const { return _addr_len; }
	HttpRequest&				getRequest() { return _request; }
	HttpAnswer&					getAnswer() { return _answer; }
	Method*						getBodyTask() const { return _body_task; }
	ConnectionStatus			getStatus() const { return _status; }
	const transfer_data&		getTransferData() { return _data; }
//-----------------------------------------------------------------------------]
public:
	std::string					findRequestHeader(const std::string& header);
	std::string					findAnswertHeader(const std::string& header);
///////////////////////////////////////////////////////////////////////////////]
	/***  SETTERS  ***/
public:
	void	resetConnection();
	void	resetRequest( void ) { _request = HttpRequest(); }
	void	resetAnswer( void ) { _answer = HttpAnswer(); }
	void	setStatus(ConnectionStatus s) { _status = s; }
	void	closeFd();
//-----------------------------------------------------------------------------]
///////////////////////////////////////////////////////////////////////////////]

};

std::ostream& operator<<(std::ostream& os, const Connection& c);
std::ostream& operator<<(std::ostream& os, const Connection::transfer_data& t);

#endif