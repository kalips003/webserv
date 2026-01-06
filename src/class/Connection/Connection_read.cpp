#include "Connection.hpp"

#include "_colors.h"

#include <iostream>
#include <sstream>

#include "Tools1.hpp"
///////////////////////////////////////////////////////////////////////////////]
enum ConnectionStatus Connection::ft_read(char *buff, size_t sizeofbuff) {

    ssize_t bytes_recv = recv(_client_fd, buff, sizeofbuff - 1, 0);

    if (bytes_recv == 0) {
        std::cerr << RED "connection closed (FIN received)" RESET << std::endl;
        return CLOSED;
    }
    else if (bytes_recv < 0) {// treat as generic fail (no errno)
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return _status; // fcntl()'s fault, no data to read yet
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
    return _status;
}

/*  v2 */
void Connection::ft_read_v2(char *buff, size_t sizeofbuff) {

    ssize_t bytes_recv = recv(_client_fd, buff, sizeofbuff - 1, 0);

    if (bytes_recv == 0) {
        std::cerr << RED "connection closed (FIN received)" RESET << std::endl;
        _status = CLOSED;
        return;
    }
    else if (bytes_recv < 0) {// treat as generic fail (no errno)
        if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
            printErr(RED "recv() failed" RESET);
            _status = CLOSED;
        }
        return ; // (else) fcntl()'s fault, no data to read yet
    }

    buff[bytes_recv] = '\0';
    std::string str_buff(buff, bytes_recv);
    std::cerr << C_134 "packet received (" RESET << bytes_recv << C_134 " bytes): \n[" RESET << str_buff << C_134 "]" RESET << std::endl;

    if (_status == FIRST) {
        int err_rtrn = _request.readingFirstLine(str_buff);
        if (err_rtrn >= 100) {
            _status = create_error(err_rtrn);
            return ;
        }
        _status = static_cast<ConnectionStatus>(err_rtrn);
    }

    if (_status <= READING_HEADER) {
        int err_rtrn = _request.readingHeaders(str_buff);
        if (err_rtrn >= 100) {
            _status = create_error(err_rtrn);
            return ;
        }
        _status = static_cast<ConnectionStatus>(err_rtrn);
    }
    
    else if (_status == READING_BODY) {

        if (_request._body_size < 4096)
            _request._body += str_buff;
        else if (_request._fd_body >= 0)
            write(_request._fd_body, buff, bytes_recv); // write(_request.fd_body, buff, bytes)
        _request._body_bytes_received += bytes_recv;
        if (_request._body_bytes_received >= static_cast<size_t>(_request._body_size))
            return DOING;

    }
    return _status;
}

#include "HttpMethods.hpp"
///////////////////////////////////////////////////////////////////////////////]
/**
 * Called as long as the request is in FIRST state
 * meaning as long as the "\r\n" delimitor of the Request Line isnt found
 *
 * If delimitor is found, check if the Request Line is valid (valid Method & HTTP/1.1)
 *
 * If invalid, fills the _answer with the correct error

 * @param first_rec   Concatenation of everything received (_buffer + last recv())
 * @return      FIRST if delim not found, READING_HEADER if Request Line received in full and valid, 
 SENDING if error.
 */
enum ConnectionStatus Connection::parse_header_first_read(std::string first_rec) {

    if (first_rec.find("\r\n") == std::string::npos)
        return FIRST;

    std::stringstream ss(first_rec);
    std::string word;

    ss >> word;
    
    if (isMethodValid(word) < 0)
        return create_error(405);
    if (!(ss >> word >> word) || word != "HTTP/1.1")
        return create_error(400);
    return READING_HEADER;
}

#include <string.h>
///////////////////////////////////////////////////////////////////////////////]
// helper function to find the end of header delimitor ("\r\n\r\n")
// keep in memory where we are at in finding such delimitor in:
    // _request.header_delim_progress
// once parsing compete, push in _buffer
/**
 * Helper function to find the end of header delimitor ("\r\n\r\n")
 * keep in memory where we are at in finding such delimitor in: _header_delim_progress
 *
 *

 * @param first_rec   C
 * @return      F
 */
enum ConnectionStatus    Connection::check_buffer_for_rnrn(char *buff) {          
  
    std::string delim = "\r\n\r\n";

    while (*buff != '\0') {

        if (*buff == delim[_request._header_delim_progress]) {
            _request._header_delim_progress++;

            if (_request._header_delim_progress == delim.size()) {
                _buffer.push_back(*buff);
                buff++;
                return parse_header_wrapper(buff);
            }
        }
        else
            _request._header_delim_progress = (*buff == delim[0]);
        
        _buffer.push_back(*buff);
        buff++;
    }
    return static_cast<ConnectionStatus>(_status);
}



///////////////////////////////////////////////////////////////////////////////]
enum ConnectionStatus    Connection::parse_header_wrapper(char *buf) {

    bool    request_good = parse_header_for_syntax();
    if (!request_good)
        return create_error(400);

    _request._body_size = _request.isThereBody();
    if (_request._body_size < 0) {
        printErr(ERR9 "bad body-size");
        return create_error(400);
    }

    _request._body = std::string(buf);
    _request._body_bytes_received = _request._body.size();
    // _buffer = "";
    _buffer.clear();
    
    _body_task = createTask(_request._method, _request, _answer);
    if (_body_task->_status) {
        std::cerr << "BodyTask status: " << _body_task->_status << std::endl;
        // return create_error(_body_task->_status);
    }
    if (!_request._body_size || _request._body_bytes_received >= static_cast<size_t>(_request._body_size))
        return DOING;
    else 
        return READING_BODY;
}


#include <algorithm>
///////////////////////////////////////////////////////////////////////////////]
// takes the string containing all the headers, ending in "\r\n\r\n"
// fills the struct http_request, return false on error (parsing errors, invalid http request)
//  parse only invalid syntax, not validity of the content
bool Connection::parse_header_for_syntax() {

    std::vector<std::string> v;
    v = splitOnDelimitor(_buffer, "\r\n");
    if (!v.size())
        return printErr(ERR9 "emtpy vector"); // emtpy vector

    {
        std::vector<std::string> first_line = splitOnWhite(v[0]);

        if (first_line.size() != 3)
            return printErr(ERR9 "bad first line");

        _request._method = first_line[0];
        _request._path = first_line[1];  
        _request._version = first_line[2];
    }

    std::vector<std::string>::iterator it = v.begin();
    ++it;
    int i = 0;
    while (!(*it).empty()) {
        i++;
        size_t colon_pos = it->find(':');
        if (colon_pos == std::string::npos)
            return printErr(ERR8 "bad header?, no colon");           
            
        std::string key = trim_white(it->substr(0, colon_pos));
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        std::string value = trim_white(it->substr(colon_pos + 1));
        _request._headers[key] = value;

        ++it;
    }
    return true;
}
