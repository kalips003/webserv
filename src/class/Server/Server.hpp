#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <map>
#include <iostream>

#include "defines.hpp"
#include "Connection.hpp"
#include "ServerSettings.hpp"

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

private:
///////////////////////////////////////////////////////////////////////////////]
    struct sockaddr_in      _addr;
    // struct ServerSettings   _settings;

    int                     _socket_fd;
    bool                    _server_status;

    map_clients				_clients;
///////////////////////////////////////////////////////////////////////////////]

public:
    Server( const char* confi_file );
	~Server( void );

//-----------------------------------------------------------------------------]
private:
	bool	create_listening_socket( void );
	void	accept_client( void );

//-----------------------------------------------------------------------------]
public:
    void    run( void ) {};
    void    run_simple( void );
    void    run_better( void );

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