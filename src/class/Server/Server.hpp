#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <map>

#include "connection.hpp"
#include "ServerSettings.hpp"
///////////////////////////////////////////////////////////////////////////////]
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
    struct ServerSettings  _settings;

    int                     _socket_fd;
    bool                    _server_status;

    std::map<int, Connection>   _clients;
///////////////////////////////////////////////////////////////////////////////]

public:
///////////////////////////////////////////////////////////////////////////////]
/**
 * Constructor for the Server.
 *
 * Takes as argument a VALID char* with the path of the config file for the server.
 *
 * Once the construction is finished, _server_status holds the status of the construction (OK / NOK).
 *
 * If _server_status == OK, the server is listening and ready for accept() / epoll()
 *
 * @param confi_file   Path of config file.
 * @return         _server_status true if parsing succeeded, false otherwise.
 */
    Server( const char* confi_file );
///////////////////////////////////////////////////////////////////////////////]
    Server::~Server( void ) { if (_socket_fd >= 0) close(_socket_fd); }

///////////////////////////////////////////////////////////////////////////////]
//	RUNNING
///////////////////////////////////////////////////////////////////////////////]
    void    run( void ) {};
    void    run_simple( void );
    void    run_better( void );


///////////////////////////////////////////////////////////////////////////////]
// SETTERS / GETTERS
///////////////////////////////////////////////////////////////////////////////]
    bool			getStatus() { return _server_status; }
    bool			getfd() { return _socket_fd; }
    ServerSettings	getSettings() { return _settings; }

/**
 * Remove the given Client from the list
 *
 * @param it   std::map<int, connection> iterator to pop
 * @return     next client in the list
 */
	c_it	pop_connec(c_it it);

///////////////////////////////////////////////////////////////////////////////]
// PRIVATE
///////////////////////////////////////////////////////////////////////////////]
private:
/**
 * Create the listening socket.
 *
 * Fills the struct sockaddr_in _addr
 *
 * If return True, the socket is listening and ready to accept
 *
 * @return         FALSE on any error (and print the err msg), TRUE otherwise
 */
	bool	create_listening_socket( void );
	void	accept_client( void );
};


#endif