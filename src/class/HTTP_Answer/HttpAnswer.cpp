#include "HttpAnswer.hpp"

#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cerrno>
#include <sys/socket.h>
#include <string.h>

#include "Tools1.hpp"
#include "HttpStatusCode.hpp"

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
httpAnswer::~httpAnswer() {

	if (_fd_body >= 0) close(_fd_body);
}

// #include "HttpStatusCode.hpp"
///////////////////////////////////////////////////////////////////////////////]
/**
 * Fills the Answer with the corresponding error message

 * @param errCode   Http Status Code
 * @return         SENDING or CLOSED on error (errCode invalid)		---*/
enum ConnectionStatus    httpAnswer::create_error(int errCode) {

	std::string s = return_http_from_code(errCode);
	if (s.empty())
		return CLOSED;
	else {
		_status = errCode;
		_msg_status = s;
		_body = "<html><body style=\"background:#111;color:#eee;text-align:center;\">"
			"<h1>" + itostr(errCode) + " " + _msg_status + "</h1>"
			"<img src=\"/errors/" + itostr(errCode) + ".jpg\" alt=\"error\">"
			"</body></html>";
		_headers["Content-Type"] = "text/html";
		// _body_size = _body.size();
		// _headers["Content-Length"] = itostr(_body.size());

		if (_fd_body >= 0) {
			close(_fd_body);
			_fd_body = -1;
		}
		http_answer_ini();
		
		return SENDING;
	}
}

#include <ctime>
#include <string>
#include <algorithm>
///////////////////////////////////////////////////////////////////////////////]
/** Set default headers for an Answer		---*/
void	httpAnswer::defaultHeaders() {

	map_strstr::iterator it = _headers.find("Content-Type");
	if (it != _headers.end())
		std::replace(it->second.begin(), it->second.end(), '|', ';');

	_headers["Connection"] = "close";
	_headers["Server"] = "Webserv/0.1";
	_headers["Cache-Control"] = "no-cache"; // no-cache or max-age=3600

// DATE
	// get current time in UTC
	std::time_t t = std::time(NULL);
	std::tm tm_utc;
	gmtime_r(&t, &tm_utc); // thread-safe, fills tm_utc with UTC time

	// buffer for formatted date
	char buf[30]; // enough for "Day, DD Mon YYYY HH:MM:SS GMT"
	std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm_utc);
	_headers["Date"] = std::string(buf);


	// if (_fd_body >= 0)
		// "Last-Modified" Last modified timestamp of the file
}

///////////////////////////////////////////////////////////////////////////////]
/**	 Concactenate answer + headers into _head	---*/
void httpAnswer::http_answer_ini() {

	_head.reserve(4096);

	if (!_body.empty()) {
		_body_size = _body.size();
		_headers["Content-Length"] = itostr(_body.size());
	}
	defaultHeaders();
	_head = rtrnFistLine() + concatenateHeaders() + "\r\n";
}

// #include <sstream>
//-----------------------------------------------------------------------------]
/**  concatenate and return first line: <HTTP/1.1 200 OK\r\n>		---*/
std::string httpAnswer::rtrnFistLine() {

	std::stringstream ss;
	ss << _status;

	return _version + " " + ss.str() + " " + _msg_status + "\r\n";
}

//-----------------------------------------------------------------------------]
/**  return a string of all headers separated by '\r\n'		---*/
std::string	httpAnswer::concatenateHeaders() {

	std::string s;
	for (map_strstr::iterator it = _headers.begin(); it != _headers.end(); it++)
		s += it->first + ": " + it->second + "\r\n";

	return s;
}


// #include <sys/socket.h>
///////////////////////////////////////////////////////////////////////////////]
/**
 * Send from the correct source, using the internal send state: _sending_status
 * 
 * After sending update _head / _body
 * 
 * @param buff   Buffer to use for send()
 * @param size_buff   Size of buffer
 * @param fd_client   Fd client to send to
 * @return			SENDING or CLOSED state of the connection after the last send		---*/
enum ConnectionStatus    httpAnswer::sending(char *buff, size_t size_buff, int fd_client) {

	ssize_t bytesLoaded = fillBuffer(buff, size_buff);
	if (bytesLoaded < 0)
		return SENDING;
	if (!bytesLoaded)
		return CLOSED;

	ssize_t bytesSent = send(fd_client, buff, bytesLoaded, 0);
	if (bytesSent <= 0) {
		if (bytesSent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			return SENDING; 
		return CLOSED;
	}
	oss msg; msg << "Packet sent: loaded [" << bytesLoaded << "] sent [" << bytesSent << "]";
	if (_sending_status != SENDING_BODY_FD)
		msg << C_420 "\nPacket: {" RESET << std::string(buff, bytesSent) << C_420 "}" RESET;
	printLog(DEBUG, msg.str(), 1);

	updateAfterSend(buff, bytesLoaded, bytesSent);
	if (_sending_status == ENDED)
		return CLOSED;
	else
		return SENDING; 
}

// #include <unistd.h>
// #include <string.h>
//-----------------------------------------------------------------------------]
/**	Choose from what source to fill the buffer with
* @return bytesLoaded into buffer			---*/
ssize_t	httpAnswer::fillBuffer(char *buff, size_t size_buff) {

	ssize_t bytesLoaded = 0;

	switch (_sending_status) {

		case SENDING_HEAD :
			bytesLoaded = std::min(size_buff, _head.size());
			memcpy(buff, _head.c_str(), bytesLoaded);
			break ;

		case SENDING_BODY :
			bytesLoaded = std::min(size_buff, _body.size() - _bytes_sent);
			memcpy(buff, &(_body.c_str()[_bytes_sent]), bytesLoaded);
			break ;

		case SENDING_BODY_FD :
			if (!_leftover.empty()) {
				bytesLoaded = _leftover.size();
				memcpy(buff, _leftover.c_str(), _leftover.size());
				_leftover.clear();
			}
			else
				bytesLoaded = read(_fd_body, buff, size_buff);
			if (bytesLoaded < 0)
				printErr(C_350 "read()");
			break ;

		case ENDED :
			std::cout << C_350 "Answer ENDED" RESET << std::endl;
	}
	return bytesLoaded;
}

//-----------------------------------------------------------------------------]
/**	Compare bytesLoaded | bytesSent, manage partial send, leftover
*
*	Update the internal state _sending_status
*
*	Close the _fd_body if any and finished			---*/
void	httpAnswer::updateAfterSend(char *buff, ssize_t bytesLoaded, ssize_t bytesSent) {

	_bytes_sent += bytesSent;

	switch (_sending_status) {

		case SENDING_HEAD :
			if (bytesSent == static_cast<ssize_t>(_head.size())) {
				printLog(DEBUG, RED "HEAD cleared" RESET, 1);
				_head.clear();
				_bytes_sent = 0;
			}
			else
				_head = _head.substr(bytesSent);
			break ;

		case SENDING_BODY : 
			if (_bytes_sent == _body_size)
				_body.clear();
			break ;

		case SENDING_BODY_FD :
			if (_bytes_sent == _body_size) {
				close(_fd_body); // ------------------------------------?????????????????????????????????????????
				_fd_body = -1;
			}
			else if (bytesSent != bytesLoaded)
				_leftover = std::string(buff + bytesSent, bytesLoaded - bytesSent);
			break ;

		case ENDED :
			std::cout << C_350 "Answer ENDED" RESET << std::endl;
	}
	_sending_status = isThereBody();
	oss msg; msg << C_431 "[_sending_status]" RESET << _sending_status;
	printLog(DEBUG, msg.str(), 1);
}

///////////////////////////////////////////////////////////////////////////////]
void	httpAnswer::setFirstLine(int code) {
	_version = "HTTP/1.1";
	_status = code;
	_msg_status = return_http_from_code(code);
	if (_msg_status.empty()) {
		_status = 500;
		_msg_status = return_http_from_code(500);
	}
}

void	httpAnswer::setBodySize(size_t size) {

	_body_size = size; 
	addToHeaders("Content-Length", itostr(size));
}
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
/***  								GETTERS 								***/
///////////////////////////////////////////////////////////////////////////////]
/** find the given setting in the _headers
*	@return the string value of the setting
*
* if setting not found, return "" empty string	---*/
std::string httpAnswer::find_setting(const std::string& set) const {

	map_strstr::const_iterator it = _headers.begin();
	it = _headers.find(set);
	if (it == _headers.end()) {
		std::cerr << RED "setting not found: " RESET << set << std::endl;
		return "";
	}
	else 
		return it->second;
}

///////////////////////////////////////////////////////////////////////////////]
/**	return AnswerStatus dependinng on what type of body to send */
AnswerStatus	httpAnswer::isThereBody() const { 
	if (!_head.empty())
		return SENDING_HEAD;
	else if (_bytes_sent == _body_size)
		return ENDED;
	else if (!_body.empty() && _fd_body == -1)
		return SENDING_BODY;
	else if (_body.empty() && _fd_body >= 0)
		return SENDING_BODY_FD;
	else
		return ENDED;
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, httpAnswer& a) {

	os << C_542 "---------------------------------------------\n" RESET;
	os << C_542 "\t- ANSWER -\n\n" RESET;
	os << a.rtrnFistLine() << std::endl;
	os << C_542 "\t- HEADERS -\n" RESET;

	if (a.getHeaders().empty())
		os << C_512 "(empty)\n" RESET;
	else
		for (map_strstr::const_iterator  h = a.getHeaders().begin(); h != a.getHeaders().end(); h++) {
			os << C_114 << h->first << RESET ": " << h->second << std::endl;
		}
	os << C_542 "\t- BODY -\n[" RESET;
	if (!a.getHead().empty()) {
		os << C_542 "head_size:\n(" RESET << a.getHead().size() << C_542 ")" RESET << std::endl;
		os << C_542 "head:\n[" RESET << a.getHead() << C_542 "]" RESET << std::endl;
	}
	else
		os << C_542 "head: (empty)\n[" RESET << std::endl;
	if (!a.getBodyLeftover().empty()) {
		os << C_542 "body_leftover_size:\n(" RESET << a.getHead().size() << C_542 ")" RESET << std::endl;
		os << C_542 "body_leftover:\n[" RESET << a.getBodyLeftover() << C_542 "]" RESET << std::endl;
	}
	if (a.getBodyFd() >= 0) {
		os << C_542 "\t- FILE BODY -\n[" RESET;
		os << C_542 "fd_body: " RESET << a.getBodyFd() << std::endl;
	}
	else 
		os << C_542 "\t- NO FILE BODY -\n[" RESET;
	os << C_333 "_full_size: " RESET << a.getFullSize() << std::endl;
	os << C_333 "_bytes_sent: " RESET << a.getBytesSent() << std::endl;
	os << C_542 "---------------------------------------------\n" RESET;
	return os;
}
