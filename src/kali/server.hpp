#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>

#include "_colors.h"

#include "structs.hpp"
#include "webserv.hpp"

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
class Server {

private:
    struct sockaddr_in      _addr;
    struct server_settings  _settings;

    int                     _socket_fd;
    bool                    _server_status;

public:
    Server( const char* confi_file );
    ~Server( void );


    void    run( void ) {};
    void    run_simple( void );
    bool    getStatus() { return _server_status; }
};


#endif