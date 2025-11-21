#include "webserv.hpp"

///////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
Task* createTask(const std::string& method, http_request& req, http_answer& ans) {
    if (method == "GET")       return new Ft_get(req, ans);
    // else if (method == "POST") return new Ft_post(req, ans);
    // else if (method == "PUT")  return new Ft_put(req, ans);
    // ... other methods
    else return NULL;  // unknown method → 405 or reject
}


std::ostream& operator<<(std::ostream& os, const connection& c) {

    os << C_152 "Client n." RESET << c._client_fd;
    // os << C_152 "Client n." RESET << c._client_fd << std::endl;
    // os << C_434 "addr: " RESET << c._client_addr.sin_addr.s_addr << std::endl;
    return os;
}


#include <fcntl.h>
///////////////////////////////////////////////////////////////////////////////]
// NON BLOCCKING recv():
//      int     fcntl(int fd, int cmd, ... /* arg */ );
// F_GETFL = get current file status flags (O_NONBLOCK, O_APPEND, ...).
//          return bitmask of flags
// F_SETFL = set flags with 'arg'
//          return ?
// FLAGS:
/*
O_NONBLOCK	make reads/writes non-blocking
O_APPEND	always write at the end of file
O_SYNC	    write operations wait for physical completion
O_ASYNC	    enable SIGIO delivery when I/O is possible (less used)
*/
bool    set_flags(int fd_to_set, int flag_to_add) {

    int flags = fcntl(fd_to_set, F_GETFL, 0);
    if (flags < 0){
        printErr(RED "fcntl( F_GETFL ) failed" RESET);
        return false;
    }
    if (fcntl(fd_to_set, F_SETFL, flags | flag_to_add) < 0){
        printErr(RED "accept( F_SETFL ) failed" RESET);
        return false;
    }
    return true;
}
