#include "webserv.hpp"

#include <unistd.h>
// #include <cstring>
///////////////////////////////////////////////////////////////////////////////]
enum ConnectionStatus    connection::create_error(int err_code) {

    std::string s = return_http_from_code(err_code);
    if (s.empty())
        return CLOSED;
    else {
        _answer.status = err_code;
        _answer.msg_status = s;
        _answer.head = "<html><body><h1>" + itostr(err_code) + " " + _answer.msg_status + "</h1></body></html>";
        _answer._headers["Content-Type"] = "text/html";
        _answer._headers["Content-Length"] = itostd(_answer.head.size());
        _answer._full_size = 0;
        if (_answer.fd_body >= 0) {
            close(_answer.fd_body);
            _answer.fd_body = -1;
        }
        _answer.http_answer_ini();
        return SENDING;
    }
}
