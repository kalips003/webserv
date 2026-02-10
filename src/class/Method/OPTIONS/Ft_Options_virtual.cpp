#include "Ft_Options.hpp"

#include "Log.hpp"
#include "Tools1.hpp"
#include "Tools2.hpp"
#include "HttpAnswer.hpp"
#include "HttpRequest.hpp"

///////////////////////////////////////////////////////////////////////////////]
void Ft_Options::printHello() {
	LOG_DEBUG("OPTIONS method called");
}

///////////////////////////////////////////////////////////////////////////////]
/**	rtrn_open == 404 / 403
* In Post, reject file existance?  */
// POST /images/file
// > path/to/root/file
int		Ft_Options::howToHandleFileNotExist(const std::string& ressource, int rtrn_open) {
	(void)ressource;
	(void)rtrn_open;

	std::string a_methods;
	for (size_t i = 0; i < _location_block->data.allowed_methods.size(); ++i) {
		a_methods += _location_block->data.allowed_methods[i];
		if (i + 1 != _location_block->data.allowed_methods.size())
			a_methods += ", ";
	}

	_answer.addToHeaders("Allow", a_methods);
	return Connection::SENDING;
}

///////////////////////////////////////////////////////////////////////////////]
int		Ft_Options::handleFileExist(std::string& path) {
	(void)path;

	std::string a_methods;
	for (size_t i = 0; i < _location_block->data.allowed_methods.size(); ++i) {
		a_methods += _location_block->data.allowed_methods[i];
		if (i + 1 != _location_block->data.allowed_methods.size())
			a_methods += ", ";
	}

	_answer.addToHeaders("Allow", a_methods);
	return Connection::SENDING;
}

///////////////////////////////////////////////////////////////////////////////]
/** */
int		Ft_Options::handleDir(std::string& ressource) {
	(void)ressource;

	std::string a_methods;
	for (size_t i = 0; i < _location_block->data.allowed_methods.size(); ++i) {
		a_methods += _location_block->data.allowed_methods[i];
		if (i + 1 != _location_block->data.allowed_methods.size())
			a_methods += ", ";
	}

	_answer.addToHeaders("Allow", a_methods);
	return Connection::SENDING;
}

#include <cstdlib> 
#include "HttpRequest.hpp"
///////////////////////////////////////////////////////////////////////////////]
/** */
void	Ft_Options::prepareChild(const std::string& ressource, const std::string& query) {

	setenv("REQUEST_METHOD", "OPTIONS", 1);
	setenv("QUERY_STRING", query.c_str(), 1);
	setenv("SCRIPT_FILENAME", ressource.c_str(), 1);
	setenv("REDIRECT_STATUS", "200", 1);

	std::string content_length = itostr(_request.getFile().getBodySize());
	setenv("CONTENT_LENGTH", content_length.c_str(), 1);
	const std::string* content_type = _request.find_in_headers("content-type");
	if (!content_type)
		LOG_WARNING("CGI OPTIONS, Content-Type missing");
	setenv("CONTENT_TYPE", (*content_type).c_str(), 1);
	setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	
	int fd_body_tmp = _request.getFile()._fd;
	if (fd_body_tmp >= 0) {
		lseek(fd_body_tmp, 0, SEEK_SET);
		dup2(fd_body_tmp, STDIN_FILENO);
		close(fd_body_tmp);
	}
}