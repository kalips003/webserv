#include "webserv.hpp"

#include "Server.hpp"

///////////////////////////////////////////////////////////////////////////////]
void    Server::ft_exec(connection& connec) {

    switch(isMethodValid(connec._request.method)) {

        case GET: ft_get(); break;
        case POST: ft_post(); break;
        case PUT: ft_put(); break;
        // ...
        default: ft_invalid(); break;
    }
}