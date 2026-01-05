#include "Connection.hpp"

#include <unistd.h>

#include "HttpStatusCode.hpp"
#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
/**
 * Set flags for the given fd

 * @param fd_to_set   FD to set flags to
 * @param flag_to_add   OR sum of the flags to add
 * @return         SENDING or CLOSED on error
 */
enum ConnectionStatus    Connection::create_error(int err_code) {

    std::string s = return_http_from_code(err_code);
    if (s.empty())
        return CLOSED;
    else {
        _answer._status = err_code;
        _answer._msg_status = s;
        _answer._head = "<html><body><h1>" + itostr(err_code) + " " + _answer._msg_status + "</h1></body></html>";
        _answer._headers["Content-Type"] = "text/html";
        _answer._headers["Content-Length"] = itostr(_answer._head.size());
        _answer._full_size = 0;
        if (_answer._fd_body >= 0) {
            close(_answer._fd_body);
            _answer._fd_body = -1;
        }
        _answer.http_answer_ini();
        return SENDING;
    }
}

