#ifndef SERVER_HPP
#define SERVER_HPP

#include "Log.hpp"
#include <netinet/in.h>
#include <map>
#include <iostream>
#include <sys/epoll.h> 

#include "defines.hpp"
#include "Connection.hpp"
#include "Settings.hpp"
#include "Cookies.hpp"

extern bool g_ServerEnd;

#define EPOLL_TIMEOUT 1000

///////////////////////////////////////////////////////////////////////////////]
typedef std::map<int, Connection> map_clients;
typedef std::map<int, Connection>::iterator c_it;
///////////////////////////////////////////////////////////////////////////////]
class Server {

public:
//-----------------------------------------------------------------------------]
	enum ConnectionAcceptResult {
		ACCEPT_OK = 1,        // one client accepted
		ACCEPT_EMPTY = 0,     // no more clients (EAGAIN)
		ACCEPT_RETRY = -1,    // EINTR / ECONNABORTED
		ACCEPT_FATAL = -2     // EMFILE / ENFILE / etc.
	};

public:
///////////////////////////////////////////////////////////////////////////////]
	struct server_listen {
		int								_socket_fd;
		int								_listen_port;
		struct sockaddr_in				_addr;
		const Settings::server_setting&	_settings;

		server_listen(const Settings::server_setting& settings) : _settings(settings) {}
	};

private:
///////////////////////////////////////////////////////////////////////////////]
	int								_epoll_fd;
	struct epoll_event				_events[MAX_EVENTS];
	std::map<int, server_listen>	_sockets;

	map_clients						_clients;

	std::map<std::string, Cookies>	_cookies;

	bool							_server_status;
///////////////////////////////////////////////////////////////////////////////]

public:
	Server( const char* confi_file );
	~Server( void );

//-----------------------------------------------------------------------------]
private:
	bool					create_all_listening_socket( void );	
	static bool				create_listening_socket(Server::server_listen& new_socket);
	bool					create_epoll( void );
	bool					accept_new_clients(void* ptr, char *buff, size_t sizeofbuff);
	void					accept_clients(char *buff, size_t sizeofbuff, server_listen* this_domain);
    ConnectionAcceptResult	accept_one_client(char *buff, size_t sizeofbuff, server_listen* this_domain);
//-----------------------------------------------------------------------------]
public:
	void	run( void );
private:
	void	handle_EPOLLERR(Connection* client);
	void	handle_EPOLLRDHUP(Connection* client);
	void	handle_EPOLLHUP(Connection* client, char* buffer, size_t sizeofbuff);
public:
	void	reboot( void );
//-----------------------------------------------------------------------------]
public:
	c_it	pop_connec(c_it it, bool timeout);

public:
//-----------------------------------------------------------------------------] 
///////////////////////////////////////////////////////////////////////////////]
/***  GETTERS  ***/
	bool	getStatus() { return _server_status; }
/***  SETTERS  ***/

///////////////////////////////////////////////////////////////////////////////]

	friend std::ostream&	operator<<(std::ostream& oss, Server& s);

};

bool	init_signals(void);

std::ostream&	operator<<(std::ostream& oss, Server& s);

#endif