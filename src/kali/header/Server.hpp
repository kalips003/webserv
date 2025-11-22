#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>

#include "webserv.hpp"

#include "connection.hpp"
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
class Server {

private:
    struct sockaddr_in      _addr;
    struct server_settings  _settings;

    int                     _socket_fd;
    bool                    _server_status;

    std::map<int, connection>   _clients;

public:
    Server( const char* confi_file );
    ~Server( void );

    void    run( void ) {};
    void    run_simple( void );
    void    run_better( void );
    bool    getStatus() { return _server_status; }
    server_settings    getSettings() { return _settings; }
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