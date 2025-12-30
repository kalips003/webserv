#include "Answer.hpp"

#include <unistd.h>

///////////////////////////////////////////////////////////////////////////////]
// http_answer
///////////////////////////////////////////////////////////////////////////////]
httpAnswer::~httpAnswer() {

    if (fd_body >= 0) close(fd_body);
}

///////////////////////////////////////////////////////////////////////////////]
bool    httpAnswer::addToHeaders(std::string& parameter, std::string& value) {
    _headers[parameter] = value;
    return true;
//  temp, need to make better   
    map_strstr::iterator it = _headers.begin();
}


///////////////////////////////////////////////////////////////////////////////]
/*  return a string of all headers separated by '\r\n' */
static std::string    concatenateHeaders(map_strstr& headers) {
    std::string s;
    for (map_strstr::const_iterator it = headers.begin(); it != headers.end(); it++)
        s += it->first + ": " + it->second + "\r\n";

    return s;
}

std::string itostr(int n);
///////////////////////////////////////////////////////////////////////////////]
// concactenate answer + headers into _body
// _full_size is updated with the added _head
// if _head alreader contain a small body, head is inserted in front
void httpAnswer::http_answer_ini() {

    std::string     headers;
    headers.reserve(4096); // optional
    headers = rtrnFistLine() + concatenateHeaders(_headers) + "\r\n";

    _head.reserve(headers.size() + _head.size());
    _head.insert(0, headers);
    _full_size += _head.size();
}

#include <sstream>
///////////////////////////////////////////////////////////////////////////////]
std::string httpAnswer::rtrnFistLine() {
    std::stringstream ss;
    ss << _status;

    return _version + ss.str() + _msg_status + "\r\n";
}

#include <iostream>
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, httpAnswer& r) {

    os << C_542 "---------------------------------------------\n" RESET;
    os << C_542 "\t- ANSWER -\n\n" RESET;
    os << r.rtrnFistLine() << std::endl;
    os << C_542 "\t- HEADERS -\n" RESET;

    if (r._headers.empty())
        os << C_512 "(empty)\n" RESET;
    else
        for (map_strstr::iterator  h = r._headers.begin(); h != r._headers.end(); h++) {
            os << C_114 << h->first << RESET ": " << h->second << std::endl;
        }
    os << C_542 "\t- BODY -\n[" RESET;
    if (!r._head.empty()) {
        os << C_542 "head_size:\n(" RESET << r._head.size() << C_542 ")" RESET << std::endl;
        os << C_542 "head:\n[" RESET << r._head << C_542 "]" RESET << std::endl;
    }
    else
        os << C_542 "head: (empty)\n[" RESET << std::endl;
    if (!r.body_leftover.empty()) {
        os << C_542 "body_leftover_size:\n(" RESET << r._head.size() << C_542 ")" RESET << std::endl;
        os << C_542 "body_leftover:\n[" RESET << r.body_leftover << C_542 "]" RESET << std::endl;
    }
    if (r.fd_body >= 0) {
        os << C_542 "\t- FILE BODY -\n[" RESET;
        os << C_542 "fd_body: " RESET << r.fd_body << std::endl;
    }
    else 
        os << C_542 "\t- NO FILE BODY -\n[" RESET;
    os << C_333 "_full_size: " RESET << r._full_size << std::endl;
    os << C_333 "_bytes_sent: " RESET << r._bytes_sent << std::endl;         
    os << C_542 "---------------------------------------------\n" RESET;
    return os;       
}
