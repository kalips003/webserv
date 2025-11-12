#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>

#include "webserv.hpp"

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
    void    run_simple_v2( void );
    bool    getStatus() { return _server_status; }
    bool    getfd() { return _socket_fd; }
    bool    pop_fd(int fd);



// STATIC
private:
    static bool create_listening_socket(Server& dis);
};


#endif