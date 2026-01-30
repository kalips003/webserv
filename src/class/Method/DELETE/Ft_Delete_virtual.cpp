#include "Ft_Delete.hpp"

#include "Log.hpp"
#include "Tools1.hpp"
#include "Tools2.hpp"
#include "HttpAnswer.hpp"

///////////////////////////////////////////////////////////////////////////////]
void Ft_Delete::printHello() {
	LOG_DEBUG("DELETE method called");
}

#include <sys/wait.h>
///////////////////////////////////////////////////////////////////////////////]
/**	Funciton called on second loop, once _cgi_status == CGI_DOING, 
* child is set-up and so on 
* @return -1 if cgi still going 
* @return Connection::SENDING if cgi finished (and handled)
* @return ErrCode in the case of any error
*/
int		Ft_Delete::exec_cgi() {
	LOG_ERROR("--> you have to do this part DELETE (execcgi)");

//
	// Implementation
	// return -1;
//



// if implementation finished:
	// setCGIStatus() = CGI_NONE;

	// int code_rtrn_child;
	// waitpid(_cgi_data._child_pid, &code_rtrn_child, 0);
	// _cgi_data._child_pid = -1;

	// close(_cgi_data._child_pipe_fd);
	// _cgi_data._child_pipe_fd = -1;

	// epollChangeFlags(_data._epoll_fd, _data._client_fd, _data._this_ptr, EPOLL_CTL_ADD);
	// return errCode;
	return Connection::SENDING;
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
int		Ft_Delete::handleFile(std::string& path) {

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
	const std::string* content_type = getRequest().find_setting("content-type");
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