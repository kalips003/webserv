#include "Ft_Get.hpp"

///////////////////////////////////////////////////////////////////////////////]
void Ft_Get::printHello() {
	LOG_DEBUG("GET method called");
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
int		Ft_Get::handleFileExist(std::string& path) {

	if (access(path.c_str(), R_OK) != 0) { // even if file exist, might not be readable by server
		LOG_DEBUG("handleFile(): access(): 403");
		return 403;
	}

	if (!_answer.getTempFile().openFile(path, O_RDONLY, false)) {
		LOG_DEBUG("_answer.getTempFile().openFile(): 500");
		return 500;
	}

	_answer.setMIMEtype(path);
	return Connection::SENDING;
}

///////////////////////////////////////////////////////////////////////////////]
/** */
int		Ft_Get::handleDir(std::string& ressource) {

	if (access(ressource.c_str(), X_OK) != 0) { // even if folder exist, we neeed the rights to traverse it
		LOG_ERROR(ressource.c_str());
		return 403;
	}

	int rtrn = 404;
	std::string default_file;
	struct stat stat_default;
	std::vector<std::string> allowed_index = _location_block->data.index;
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
		return handleFileExist(default_file);
	}

// only 404, try autoindexing
	if (_location_block->data.autoindex == true)
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
 
 // replace stdin with /dev/null
	int fd_null = open("/dev/null", O_RDONLY);
	if (fd_null >= 0) {
		dup2(fd_null, STDIN_FILENO);  
		close(fd_null);
	}
	setenv("REQUEST_METHOD", "GET", 1);
	setenv("QUERY_STRING", query.c_str(), 1);
	setenv("SCRIPT_FILENAME", ressource.c_str(), 1);
	setenv("REDIRECT_STATUS", "200", 1);
}