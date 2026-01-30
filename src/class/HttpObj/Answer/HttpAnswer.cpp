#include "HttpAnswer.hpp"

#include "HttpStatusCode.hpp"

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
/**	Fills the answer from the errCode given, initialize it also */
void	HttpAnswer::createError(int errCode) {

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
		_tmp_file.~temp_file();
	
	initializationBeforeSend();
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
/**	 Concactenate answer + headers into _head	---*/
void HttpAnswer::initializationBeforeSend() {

	oss first; first << _version << " " << _status_num << " " << _status_msg;
	_first = first.str();

	_bytes_total = isThereBody();
	if (_bytes_total)
		_headers["content-length"] = itostr(_bytes_total);

	setDefaultHeaders();
	concatenateIntoHead();
}

//-----------------------------------------------------------------------------]
/** Parse the content of _buffer into > _method, _path, _version
// @return 0 for success, errCode else	---*/
int HttpAnswer::isFirstLineValid(int fd) {

	std::stringstream ss(_buffer); // "HTTP/1.1 201 Created"
	std::string word;

	if (!(ss >> word) || word != "HTTP/1.1") { // ??? - Handle other versions?
		oss msg; msg << RED "Invalid Version: " RESET << word; printLog(ERROR, msg.str(), 1);
		return 505;
	}
	_version = word;

	if (!(ss >> word)) {
		oss msg; msg << RED "Invalid Answer Status: " RESET << word; printLog(ERROR, msg.str(), 1);
		return 501; // or 400
	}
	int code; 
	if (!atoi_v2(word, code) || return_http_from_code(code).empty()) {
		oss msg; msg << RED "Invalid Answer Status: " RESET << word; printLog(ERROR, msg.str(), 1);
		return 501; // or 400
	}
	_status_num = code;
	
	_status_msg = ss.str();
	if (!(ss >> word)) {
		oss msg; msg << RED "No context Message" RESET; printLog(ERROR, msg.str(), 1);
		return 400;
	}

	oss msg; msg << "[#" << printFd(fd) << "] → " << _version << " " << code << "(" << _status_msg << ")"; printLog(LOG, msg.str(), 1);

	return 0;
}