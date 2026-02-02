#include "Ft_Delete.hpp"

#include "Log.hpp"
#include "Tools1.hpp"
#include "Tools2.hpp"
#include "HttpAnswer.hpp"

///////////////////////////////////////////////////////////////////////////////]
void Ft_Delete::printHello() {
	LOG_DEBUG("DELETE method called");
}

///////////////////////////////////////////////////////////////////////////////]
/**	rtrn_open == 404 / 403
* In Post, reject file existance?  */
int		Ft_Delete::howToHandleFileNotExist(const std::string& ressource, int rtrn_open) {
	(void)ressource;
	return rtrn_open;
}

///////////////////////////////////////////////////////////////////////////////]
// DELETE /file
/** */
int		Ft_Delete::handleFileExist(std::string& path) {

//	/path/to/to_delete/file > /path/to/to_delete
	std::string folder_path = path.substr(0, path.find_last_of("/"));

	if (folder_path.empty()) // only happen if location_root == '/'
		folder_path = "/"; // delete a file in "/"

	if (access(folder_path.c_str(), W_OK | X_OK) != 0) // need write / exec permissions
		return 403;

	if (unlink(path.c_str()) < 0) {
		LOG_ERROR("unlink()");
		return 500;
	}
	
	return Connection::SENDING;
}

///////////////////////////////////////////////////////////////////////////////]
/** */
int		Ft_Delete::handleDir(std::string& ressource) {
	(void)ressource;
	return 403; // or 405 with header {Allow: GET, HEAD}
}

#include "HttpRequest.hpp"
#include <cstdlib> 
///////////////////////////////////////////////////////////////////////////////]
/** */
void	Ft_Delete::prepareChild(const std::string& ressource, const std::string& query) {

	setenv("REQUEST_METHOD", "DELETE", 1);
	setenv("QUERY_STRING", query.c_str(), 1);
	setenv("SCRIPT_FILENAME", ressource.c_str(), 1);
	setenv("REDIRECT_STATUS", "200", 1);
	std::string content_length = itostr(getRequest().getFile().getBodySize());
	setenv("CONTENT_LENGTH", content_length.c_str(), 1);
	const std::string* content_type = getRequest().find_in_headers("content-type");
	if (!content_type)
		LOG_WARNING("CGI DELETE, Content-Type missing");
	setenv("CONTENT_TYPE", (*content_type).c_str(), 1);
	setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
	setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
	
	int fd_body_tmp = getRequest().getFile()._fd;
	if (fd_body_tmp >= 0) {
		lseek(fd_body_tmp, 0, SEEK_SET);
		dup2(fd_body_tmp, STDIN_FILENO);
		close(fd_body_tmp);
	}
}