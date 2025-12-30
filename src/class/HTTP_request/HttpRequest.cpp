#include "HttpRequest.hpp"

#include "_colors.h"
#include <iostream>
///////////////////////////////////////////////////////////////////////////////]
std::string HttpRequest::find_setting(const std::string& set) const {

    map_strstr::const_iterator it = _headers.begin();
    it = _headers.find(set);
    if (it == _headers.end()) {
        std::cerr << RED "setting not found: " RESET << set << std::endl;
        return "";
    }
    else 
        return it->second;    
}

#include <netinet/in.h>

///////////////////////////////////////////////////////////////////////////////]
enum ConnectionStatus HttpRequest::ft_read(char *buff, size_t sizeofbuff, int client_fd) {

    ssize_t bytes_recv = recv(client_fd, buff, sizeofbuff - 1, 0);

    // std::cerr << C_425 "bytes received: " << bytes << std::endl;
    if (bytes_recv == 0) { //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<????>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        std::cerr << RED "connection closed (FIN received)" RESET << std::endl;
        return CLOSED;
    }
    else if (bytes_recv < 0) {// treat as generic fail (no errno)
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return static_cast<ConnectionStatus>(_status); // fcntl()'s fault, no data to read yet
        printErr(RED "recv() failed" RESET);
        return CLOSED; //?
        // else if (errno == EINTR)
        //     return -2; // interrupted by signal, retry
    }

    buff[bytes_recv] = '\0';
    std::string str_buff(buff, bytes_recv);
    std::cerr << C_134 "packet received (" RESET << bytes_recv << C_134 " bytes): \n[" RESET << str_buff << C_134 "]" RESET << std::endl;

    if (_status == FIRST)
        _status = parse_header_first_read(_buffer + str_buff);

    if (_status <= READING_HEADER)
        _status = check_buffer_for_rnrn(buff);
    else if (_status == READING_BODY) {

        if (_request._body_size < 4096)
            _request._body += str_buff;
        else if (_request._fd_body >= 0)
            write(_request._fd_body, buff, bytes_recv); // write(_request.fd_body, buff, bytes)
        _request._body_bytes_received += bytes_recv;
        if (_request._body_bytes_received >= static_cast<size_t>(_request._body_size))
            return DOING;

    }
    return static_cast<ConnectionStatus>(_status);
}