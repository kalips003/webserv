#include "Ft_Get.hpp"


#include "Log.hpp"
#include "Tools1.hpp"
#include "Tools2.hpp"
#include "HttpAnswer.hpp"
///////////////////////////////////////////////////////////////////////////////]
void Ft_Get::printHello() {
	LOG_DEBUG("GET method called");
}

/**	Funciton called on second loop, once _cgi_status == CGI_DOING, 
* child is set-up and so on 
* @return -1 if cgi still going 
* @return Connection::SENDING if cgi finished (and handled)
* @return ErrCode in the case of any error
*/
int		Ft_Get::exec_cgi() {
	LOG_ERROR("--> you have to do this part GET (execcgi)");


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
* In Get, reject file non existance  */
int		Ft_Get::howToHandleFileNotExist(const std::string& ressource, int rtrn_open) {
	(void)ressource;
	return rtrn_open;
}

#include <fcntl.h>
///////////////////////////////////////////////////////////////////////////////]
/** */
int		Ft_Get::handleFile(std::string& path) {

	if (access(path.c_str(), R_OK) != 0) { // even if file exist, might not be readable by server {
		LOG_DEBUG("handleFile(): access(): 403");
		return 403;
	}

	if (!getAnswer().getTempFile().openFile(path, O_RDONLY, false)) {
		LOG_DEBUG("getAnswer().getTempFile().openFile(): 500");
		return 500;
	}

	getAnswer().setMIMEtype(path);
	return Connection::SENDING;
}

///////////////////////////////////////////////////////////////////////////////]
/** */
int		Ft_Get::handleDir(std::string& ressource) {

	if (access(ressource.c_str(), X_OK) != 0) { // even if folder exist, we neeed the rights to traverse it
		printErr(ressource.c_str());
		return 403;
	}

	int rtrn = 404;
	std::string default_file;
	struct stat stat_default;
	std::vector<std::string> allowed_index = getLocationBlock()->data.index;
	for (std::vector<std::string>::const_iterator it = allowed_index.begin(); it != allowed_index.end(); ++it) {
		default_file = ressource + "/" + *it; // /folder/index.html

		rtrn = isFileNOK(default_file, stat_default);
		if (!rtrn)
			break ; // server default index
		else if (rtrn == 403)
			return 403;
	}

// file exist, serve it
	if (!rtrn) {
		LOG_DEBUG("Default file found: (" << default_file << ")");
		return handleFile(default_file);
	}

// only 404, try autoindexing
	if (getLocationBlock()->data.autoindex == true)
		return serveAutoIndexing(ressource);
	else {
		LOG_WARNING("Requested folder (" << ressource << ") exist, but Autoindexing is off");
		return 403;
	}
}

#include <cstdlib> 
///////////////////////////////////////////////////////////////////////////////]
/** */
void	Ft_Get::prepareChild(const std::string& ressource, const std::string& query) {

	setenv("REQUEST_METHOD", "GET", 1);
	setenv("QUERY_STRING", query.c_str(), 1);
	setenv("SCRIPT_FILENAME", ressource.c_str(), 1);
	setenv("REDIRECT_STATUS", "200", 1);
}