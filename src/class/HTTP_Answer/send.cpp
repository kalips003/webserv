#include "Answer.hpp"

#include <unistd.h>
#include <cstring>
#include <cerrno>

#include <iostream>
#include <sys/socket.h>
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
// v 2.0
/*
    Load buffer buff from 3 different sources, by priority:
        1) _head string
        2) leftovers from previous partial send
        3) content of file body (fd) if any
    one call is one single send()
    increment _bytes_sent
    return the status of the connection
*/
enum ConnectionStatus httpAnswer::sendToBuffer(int fd, char* buff, size_t sizeofbuff) {

    ssize_t bytes_to_send;
    ssize_t bytes_loaded = 0;

    if (!_head.empty()) { // CASE: _head to send first
        bytes_to_send = std::min(_head.size() - _bytes_sent, sizeofbuff);
        memcpy(buff, _head.c_str() + _bytes_sent, bytes_to_send);
    }
    else {
        if (!body_leftover.empty()) { // CASE: some leftover from previous send
            bytes_to_send = body_leftover.size();
            memcpy(buff, body_leftover.c_str(), bytes_to_send); // load and send leftover
        }
        else if (fd_body >= 0) { // CASE: there is a body
            bytes_to_send = std::min(_full_size - _bytes_sent, sizeofbuff); 

            bytes_loaded = read(fd_body, buff, bytes_to_send);
            if (bytes_loaded < 0)
                std::cerr << C_511 "errrrooorr heere" RESET << std::endl; //error
            else if (bytes_loaded < bytes_to_send)
                std::cerr << C_511 "errrrooorr heere" RESET << std::endl; //error
        }
    }

    std::cerr << C_234 "Sending:" << std::endl;
    std::cerr << "   bytes_to_send: " RESET << bytes_to_send << std::endl;
    std::cerr << C_234 "   _head.size: " RESET << _head.size() << std::endl;
    std::cerr << C_234 "   _full_size: " RESET << _full_size << std::endl << std::endl;
    std::cerr << C_234 "[" RESET<< std::string(buff, bytes_to_send) << RESET "]" << std::endl;
//
    ssize_t bytes_sent = send(fd, buff, bytes_to_send, 0);
    if (bytes_sent <= 0) {
        if (bytes_sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return SENDING; 
        return CLOSED;
    }
    std::cerr << C_234 "Bytes sent: " RESET << bytes_sent << std::endl;
//
    _bytes_sent += bytes_sent;

    if (_head.empty()) {
        if (bytes_sent < bytes_to_send) {
            buff[bytes_to_send] = '\0';
            body_leftover = std::string(buff + bytes_sent);
        }
        else
            body_leftover.clear();
    }
    if (_bytes_sent == _head.size())
        _head.clear();
    if (_bytes_sent >= _full_size)
        return CLOSED;
    return SENDING;
}
