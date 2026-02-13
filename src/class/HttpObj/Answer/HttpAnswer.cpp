#include "HttpAnswer.hpp"

#include "HttpStatusCode.hpp"

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
/**	Fills the answer from the errCode given, initialize it also */
void	HttpAnswer::createError(int errCode, const std::string& method) {

	std::string s = return_http_from_code(errCode);
	if (s.empty())
		return ;

	_status_num = errCode;
	_status_msg = s;

	_headers["content-type"] = "text/html| charset=utf-8";
	// _headers["Content-Length"] set up in http_answer_ini();

	_body = "<html><body style=\"background:#111;color:#eee;text-align:center;\">"
		"<h1>" + itostr(errCode) + " " + _status_msg + "</h1>"
		"<img src=\"/errors/" + itostr(errCode) + ".jpg\" alt=\"error\">"
		"</body></html>";

	if (_tmp_file._fd >= 0)
		_tmp_file.closeTemp(true);
	
	initializationBeforeSend(method);
}

///////////////////////////////////////////////////////////////////////////////]
/**	 Correctly set _version & _status & _msg_status form code	---*/
void	HttpAnswer::setFirstLine(int code) {
	_version = "HTTP/1.1";
	_status_num = code;
	_status_msg = return_http_from_code(code);
	if (_status_msg.empty()) {
		_status_num = 500;
		_status_msg = return_http_from_code(500);
	}
}


///////////////////////////////////////////////////////////////////////////////]
/**	 Concactenate firstline + headers into _head	---*/
void HttpAnswer::initializationBeforeSend(const std::string& method) {

	oss first; first << _version << " " << _status_num << " " << _status_msg;
	_first = first.str();

	_bytes_total = isThereBody();
	_headers["content-length"] = itostr(_bytes_total);

	setDefaultHeaders();

	if (method == "HEAD") {
		_bytes_total = 0;
		_body.clear();
		_tmp_file.closeTemp(false);
	}
	
	concatenateIntoHead();
}

//-----------------------------------------------------------------------------]
/** Parse the content of _buffer into > _method, _path, _version
// @return 0 for success, errCode else	---*/
int HttpAnswer::isFirstLineValid(int fd) {

	std::stringstream ss(_buffer); // "HTTP/1.1 201 Created"
	std::string word;

	if (!(ss >> word) || word != "HTTP/1.1") { // ??? - Handle other versions?
		LOG_ERROR(RED "Invalid Version: " RESET << word);
		return 505;
	}
	_version = word;

	if (!(ss >> word)) {
		LOG_ERROR(RED "Invalid Answer Status: " RESET << word);
		return 501; // or 400
	}
	int code; 
	if (!atoi_v2(word, code) || return_http_from_code(code).empty()) {
		LOG_ERROR(RED "Invalid Answer Status: " RESET << word);
		return 501; // or 400
	}
	_status_num = code;
	
	_status_msg = ss.str();
	if (!(ss >> word)) {
		LOG_ERROR(RED "No context Message" RESET);
		return 400;
	}

	LOG_LOG("[#" << printFd(fd) << "] → " << _version << " " << code << "(" << _status_msg << ")");

	return 0;
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, const HttpAnswer& a) {

	os << "[ ANSWER ] " << a._version << " " << a._status_num << " " << a._status_msg << std::endl;
	os << static_cast<const HttpObj&>(a);
	return os;
}
