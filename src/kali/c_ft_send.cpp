#include "webserv.hpp"

#include <unistd.h>
#include <cstring>
#include <cerrno>
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
// v 2.0
/*
    Load buffer buff from 3 different sources, by priority:
        1) headers string
        2) leftovers from previous partial send
        3) content of file body (fd) if any
    one call is one single send()
    increment _bytes_sent
    return the status of the connection
*/
enum ConnectionStatus Connection::ft_send(char *buff, size_t sizeofbuff) {

    ssize_t bytes_to_send;
    ssize_t bytes_loaded = 0;

    if (!_answer.head.empty()) { // CASE: head to send first
        bytes_to_send = std::min(_answer.head.size() - _answer._bytes_sent, sizeofbuff);
        memcpy(buff, _answer.head           .c_str() + _answer._bytes_sent, bytes_to_send);



    }
    else {
        if (!_answer.body_leftover.empty()) { // CASE: some leftover from previous send
            bytes_to_send = _answer.body_leftover.size();
            memcpy(buff, _answer.body_leftover.c_str(), bytes_to_send); // load and send leftover
        }
        else if (_answer.fd_body >= 0) { // CASE: there is a body
            bytes_to_send = std::min(_answer._full_size - _answer._bytes_sent, sizeofbuff); 

            bytes_loaded = read(_answer.fd_body, buff, bytes_to_send);
            if (bytes_loaded < 0)
                std::cerr << C_511 "errrrooorr heere" RESET << std::endl; //error
            else if (bytes_loaded < bytes_to_send)
                std::cerr << C_511 "errrrooorr heere" RESET << std::endl; //error
        }
    }

    std::cerr << C_234 "Sending:" << std::endl;
    std::cerr << "   bytes_to_send: " RESET << bytes_to_send << std::endl;
    std::cerr << C_234 "   _answer.head.size: " RESET << _answer.head.size() << std::endl;
    std::cerr << C_234 "   _answer._full_size: " RESET << _answer._full_size << std::endl << std::endl;
    std::cerr << C_234 "[" RESET<< std::string(buff, bytes_to_send) << RESET "]" << std::endl;
//
    ssize_t bytes_sent = send(_client_fd, buff, bytes_to_send, 0);
    if (bytes_sent <= 0) {
        if (bytes_sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return SENDING; 
        return CLOSED;
    }
    std::cerr << C_234 "Bytes sent: " RESET << bytes_sent << std::endl;
//
    _answer._bytes_sent += bytes_sent;

    if (_answer.head.empty()) {
        if (bytes_sent < bytes_to_send) {
            buff[bytes_to_send] = '\0';
            _answer.body_leftover = std::string(buff + bytes_sent);
        }
        else
            _answer.body_leftover.clear();
    }
    if (_answer._bytes_sent == _answer.head.size())
        _answer.head.clear();
    if (_answer._bytes_sent >= _answer._full_size)
        return CLOSED;
    return SENDING;
}
