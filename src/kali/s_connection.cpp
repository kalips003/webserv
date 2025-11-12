#include "webserv.hpp"

///////////////////////////////////////////////////////////////////////////////]
int connection::recv_all_buffer() {
    char buff[4096];
    while (1) {

        std::cerr << RED " -- NEW LOOP --" << std::endl;
        ssize_t bytes = recv(_client_fd, buff, sizeof(buff) - 1, 0);
        std::cerr << C_425 "bytes received: " << bytes << std::endl;
        if (bytes == 0) { // client closed connection cleanly (FIN received)
            std::cerr << RED "connection finished" RESET << std::endl;
            return 0;
        }
        else if (bytes < 0) {// treat as generic fail (no errno)
            printErr(RED "recv() failed" RESET);
            return -1;
        }
        else if (bytes < static_cast<ssize_t>(sizeof(buff) - 1)) {// treat as generic fail (no errno)
            printErr(RED "finished?" RESET);
            return -1;
        }

        buff[bytes] = '\0';
        std::cerr << C_134 "msg received: \n" << buff << std::endl;
        _bytes_received += bytes;
        _buffer += buff;
    }
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
int connection::read_buffer(char *buff, size_t sizeofbuff) {

    ssize_t bytes = recv(_client_fd, buff, sizeofbuff - 1, 0);

    std::cerr << C_425 "bytes received: " << bytes << std::endl;
    if (bytes == 0) { //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<????>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        std::cerr << RED "connection closed (FIN received)" RESET << std::endl;
        if (_status == READING_BODY)
            _status = DOING;
        else
            _status = CLOSED;
        return 0;
    }
    else if (bytes < 0) {// treat as generic fail (no errno)
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return 0; // fcntl()'s fault, no data to read yet
        printErr(RED "recv() failed" RESET);
        return -1;
        // else if (errno == EINTR)
        //     return -2; // interrupted by signal, retry
    }

    buff[bytes] = '\0';
    std::string str_buff(buff, bytes);
    std::cerr << C_134 "msg received: \n" << str_buff << std::endl;

    if (_status < READING_BODY) {
        check_buffer(buff);
        return 0;
    }
    else if (_status == READING_BODY) {
        _buffer += str_buff;
        _bytes_received += bytes;
        if (_bytes_received >= _request.body_size)
            _status = DOING;
    }
    return 0;
}

#include <cstring>
///////////////////////////////////////////////////////////////////////////////]
// helper function to find the end of header delimitor ("\r\n\r\n")
// keep in memory where we are at in finding such delimitor in:
    // _request.status_delim
// for the moment return false / true <<<<<<<<<<<<<<<<<<<<<<<<<<<,,,,,,,,,,,,>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool    connection::check_buffer(char *b) {

    char *buff = b;
    const char* delim = "\r\n\r\n";
    int         size_delim = strlen(delim);
    while (*buff != '\0') {

        if (*buff == delim[_request.status_delim])
            _request.status_delim++;
        else
            _request.status_delim = (*buff == delim[0]);
        
        if (_request.status_delim == size_delim) {
            buff++;
            while (b != buff) {
                _buffer.push_back(*b);
                b++;
            }
            return parse_header_wrapper(b);
        }

        _buffer.push_back(*buff);
        buff++;
    }
    if (_status == FIRST) {
        _status == READING_HEADER;
        if (!parse_header_firstline()) {
            _status == SENDING;
            create_error_answer();
        }
        return true;
    }

    return false;
}


///////////////////////////////////////////////////////////////////////////////]
bool    connection::parse_header_firstline() {

    std::stringstream ss(_buffer);
    std::string word;

    ss >> word;
    if (word != part_of_known_method)
        return false;
    if (!(ss >> word >> word) || word != "HTTP/1.1")
        return false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////]
bool    connection::parse_header_wrapper(char *buf) {

    bool    request_good = parse_header();
    if (!request_good) {
        _status = SENDING;
        create_answer(); // 400 HTTP_BAD_REQUEST
        return true; // parsing issue, request is bad
    }
    else {
        _status = DOING; // if nothing
        _status = READING; // if body
        if (_status == DOING)
            exec_request();
        _buffer = std::string(buf);
        return true;
    }
}
#include <algorithm>
///////////////////////////////////////////////////////////////////////////////]
// takes the string containing all the headers, ending in "\r\n\r\n"
// fills the struct http_request, return false on error
bool connection::parse_header() {

    std::vector<std::string> v;
    v = splitOnDelimitor(_buffer, "\r\n");
    if (!v.size())
        return printErr(ERR9 "emtpy vector"); // emtpy vector

    {
        std::vector<std::string> first_line = splitOnWhite(v[0]);

        if (first_line.size() != 3)
            return printErr(ERR9 "bad first line");

        _request.method = first_line[0];
        _request.path = first_line[1];
        _request.version = first_line[2];
    }

    std::vector<std::string>::iterator it = v.begin() + 1;
    while (*it != "") {

        size_t colon_pos = it->find(':');
        if (colon_pos == std::string::npos)
            return printErr(ERR8 "bad header, no colon");
            
        std::string key = trim_white(it->substr(0, colon_pos));
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        std::string value = trim_white(it->substr(colon_pos + 1));
        _request.headers[key] = value;

        it++;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////]
bool    connection::create_answer() {
    return true;
}