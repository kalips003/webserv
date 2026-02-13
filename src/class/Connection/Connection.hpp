#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "Log.hpp"
#include "defines.hpp"

#include <string>
#include <netinet/in.h>
#include <ctime>

#include "HttpAnswer.hpp"
#include "HttpRequest.hpp"
#include "Settings.hpp"
#include "Cookies.hpp"

class Method;
class SettingsServer;
class Connection;

#define CONNECTION_TIMEOUT 20.0

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
class Connection {

public:
//-----------------------------------------------------------------------------]
	struct transfer_data {
		int								_client_fd; // fd associated with this client connection
		int								_epoll_fd;
		Connection*						_this_ptr; // ptr to this client connection
		char*							_buffer; // Shared Server buffer
		size_t							_sizeofbuff;
		const Settings::server_setting*	_settings;
		Cookies*						_this_user;

		transfer_data() : 
			_client_fd(-1), 
			_epoll_fd(-1), 
			_this_ptr(NULL),
			_buffer(NULL),
			_sizeofbuff(0),
			_settings(NULL),
			_this_user(NULL) {}

		transfer_data(int fd, int e, Connection* c, char* buffer, size_t size, const Settings::server_setting* settings) : 
			_client_fd(fd), 
			_epoll_fd(e), 
			_this_ptr(c), 
			_buffer(buffer),
			_sizeofbuff(size),
			_settings(settings),
			_this_user(NULL) {}

		friend std::ostream& operator<<(std::ostream& os, const transfer_data& t);
	};

//-----------------------------------------------------------------------------]
	enum ConnectionStatus {
	
		READING,
		DOING,
		DOING_CGI,
		SENDING,
		FINISHED,
		CLOSED
	};

///////////////////////////////////////////////////////////////////////////////]
private:
	struct sockaddr_in  			_client_addr; // struct with informations about the client
	socklen_t						_addr_len;

	const Settings::server_setting*	_settings;
	HttpRequest						_request;
	HttpAnswer						_answer;

	Method*							_body_task;

	ConnectionStatus				_status;
	transfer_data					_data;
	timeval 						_last_active;
	std::map<std::string, Cookies>& _cookies;
	Cookies*						_this_user;
///////////////////////////////////////////////////////////////////////////////]

public:
	Connection(int fd, 
				int epoll, struct sockaddr_in c, socklen_t al, 
				char* buffer, size_t size, 
				const Settings::server_setting* settings, std::map<std::string, Cookies>& cookies) :
		_client_addr(c), 
		_addr_len(al), 
		_settings(settings),
		_request(_settings),
		_answer(_settings),
		_body_task(NULL), 
		_status(READING), 
		_data(fd, epoll, this, buffer, size, settings),
		_cookies(cookies),
		_this_user(NULL) { updateTimeout(); }

	Connection(const Connection& other) :
		_client_addr(other._client_addr), 
		_addr_len(other._addr_len), 
		_settings(other._settings),
		_request(_settings),
		_answer(_settings),
		_body_task(NULL), 
		_status(READING), 
		_data(other._data),
		_cookies(other._cookies),
		_this_user(other._this_user) { _data._this_ptr = this; updateTimeout(); }

	~Connection();

int	handle_cookies();

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

	bool						checkTimeout(const timeval& now);
//-----------------------------------------------------------------------------]
public:
	std::string					findRequestHeader(const std::string& header);
	std::string					findAnswertHeader(const std::string& header);
///////////////////////////////////////////////////////////////////////////////]
	/***  SETTERS  ***/
public:
	void		updateTimeout();
	void		resetConnection();
	void		resetRequest( void ) { _request.~HttpRequest(); new (&_request) HttpRequest(_settings); }
	void		resetAnswer( void ) { _answer.~HttpAnswer(); new (&_answer) HttpAnswer(_settings); }
	void		setStatus(ConnectionStatus s) { _status = s; }
	void		closeFd();
//-----------------------------------------------------------------------------]
///////////////////////////////////////////////////////////////////////////////]

};

std::ostream& operator<<(std::ostream& os, const Connection& c);
std::ostream& operator<<(std::ostream& os, const Connection::transfer_data& t);

#endif