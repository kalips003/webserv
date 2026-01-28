#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <map>
#include <iostream>
#include <sys/epoll.h> 

#include "defines.hpp"
#include "Connection.hpp"
#include "SettingsServer.hpp"

///////////////////////////////////////////////////////////////////////////////]
typedef std::map<int, Connection> map_clients;
typedef std::map<int, Connection>::iterator c_it;
///////////////////////////////////////////////////////////////////////////////]
/**
 * Represents a full Server
 *
 * Owns a map of Clients
 */
class Server {

public:
	enum ConnectionAcceptResult {
		ACCEPT_OK = 1,        // one client accepted
		ACCEPT_EMPTY = 0,     // no more clients (EAGAIN)
		ACCEPT_RETRY = -1,    // EINTR / ECONNABORTED
		ACCEPT_FATAL = -2     // EMFILE / ENFILE / etc.
	};

private:
///////////////////////////////////////////////////////////////////////////////]
    struct sockaddr_in      _addr;
    // struct SettingsServer   _settings;

    int                     _socket_fd;
    bool                    _server_status;

    map_clients				_clients;
//-----------------------------------------------------------------------------]

    int                     _epoll_fd;
    struct epoll_event      _events[MAX_EVENTS];
///////////////////////////////////////////////////////////////////////////////]

public:
    Server( const char* confi_file );
	~Server( void );

//-----------------------------------------------------------------------------]
private:
	bool					create_listening_socket( void );
	bool					create_epoll( void );
	void					accept_clients(char *buff, size_t sizeofbuff);
    ConnectionAcceptResult	accept_one_client(char *buff, size_t sizeofbuff);
//-----------------------------------------------------------------------------]
public:
	void	run( void );
	void	run_better( void );

	bool	reset( void );
	void	reboot( void );
//-----------------------------------------------------------------------------]
public:
	c_it	pop_connec(c_it it);

public:
//-----------------------------------------------------------------------------] 
///////////////////////////////////////////////////////////////////////////////]
/***  GETTERS  ***/
    bool					getAddr() { return _server_status; }
    int						getfd() { return _socket_fd; }
    bool					getStatus() { return _server_status; }
    const map_clients&		getClients() const { return _clients; }

/***  SETTERS  ***/

///////////////////////////////////////////////////////////////////////////////]
};

std::ostream&	operator<<(std::ostream& oss, Server& s);

#endif