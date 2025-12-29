#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <map>
#include "connection.hpp"
#include "ServerSettings.hpp"
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
/**
 * Represents a full Server
 *
 * Owns a map of Clients
 */
class Server {

private:
    struct sockaddr_in      _addr;
    struct ServerSettings  _settings;

    int                     _socket_fd;
    bool                    _server_status;

    std::map<int, connection>   _clients;

public:
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
    ~Server( void );

    void    run( void ) {};
    void    run_simple( void );
    void    run_better( void );
    bool    getStatus() { return _server_status; }
    ServerSettings    getSettings() { return _settings; }
    bool    getfd() { return _socket_fd; }
    std::map<int, connection>::iterator    pop_connec(std::map<int, connection>::iterator it) {

        std::map<int, connection>::iterator next = it;
        next++;
        connection& client = it->second;

std::cerr << RED "closing: " RESET << client;
        if (client._request.headers["connection"] == "keep-alive") {
            client._request = http_request(); // clear just
            client._answer = http_answer(); // clear just
            client._buffer.clear();
            client._status = READING_HEADER;
        }
        else
            _clients.erase(it);
        return next;
    }



// STATIC
private:
    static bool create_listening_socket(Server& dis);
    void    accept_client( void );
};


#endif