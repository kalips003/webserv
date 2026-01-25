#include "Ft_Get.hpp"

#include "Tools1.cpp"
#include "Tools2.cpp"
#include "HttpAnswer.cpp"
///////////////////////////////////////////////////////////////////////////////]
void Ft_Get::printHello() {
	printLog(DEBUG, "GET method called", 1);
}

/**	Funciton called on second loop, once _cgi_status == CGI_DOING, 
* child is set-up and so on 
* @return -1 if cgi still going 
* @return 0 if cgi finished (and handled)
* @return ErrCode in the case of any error
*/
int		Ft_Get::exec_cgi() {
	printLog(ERROR, "--> you have to do this part (execcgi)", 1);
	return 0;
}
/**	rtrn_open == 404 / 403 
* In Get, reject file non existance  */
int		Ft_Get::howToHandleFileNotExist(const std::string& ressource, int rtrn_open) {
	(void)ressource;
	return rtrn_open;
}

/** */
int		Ft_Get::handleFile(std::string& path, struct stat ressource_info) {

	if (access(path.c_str(), R_OK) != 0) // even if file exist, might not be readable by server
		return 403;

	int fd = open(path.c_str(), O_RDONLY);
	if (fd < 0) {
		printErr("open()");
		return 500;
	}
	getAnswer().setFd(fd);
	getAnswer().setBodySize(ressource_info.st_size);
	getAnswer().addToHeaders("Content-Type", find_MIME_type(path));
	
	return 0;
}

/** */
int		Ft_Get::handleDir(std::string& ressource) {

	if (access(ressource.c_str(), X_OK) != 0) { // even if folder exist, we neeed the rights to traverse it
		printErr(ressource.c_str());
		return 403;
	}
											// Trailing slash edge case : '/dir' != '/dir/' = 301/302? --------------------------------< ???
	std::string ressource_indexed = ressource + *g_settings.find_setting("index");
	struct stat ressource_info2;
	int rtrn = isFileNOK(ressource_indexed, ressource_info2);
	if (rtrn) {
		if (*g_settings.find_setting("autoindex") == "on")
			return serveAutoIndexing(ressource);
		else {
			oss msg; msg << "Requested folder (" << ressource << ") exist, but Autoindexing is off";
			printLog(WARNING, msg.str(), 1);
			return 403;
		}
	}
	else {
		oss msg; msg << "Default file found: (" << ressource_indexed << ")";
		printLog(DEBUG, msg.str(), 1);
		return serveFile(ressource_indexed, ressource_info2);
	}
}

/** */
void	Ft_Get::prepareChild(const std::string& ressource, const std::string& query) {

	setenv("REQUEST_METHOD", "GET", 1);
	setenv("QUERY_STRING", query.c_str(), 1);
	setenv("SCRIPT_FILENAME", ressource.c_str(), 1);
	setenv("REDIRECT_STATUS", "200", 1);
}