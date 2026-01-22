#include "Ft_Get.hpp"
#include "_colors.h"

#include <iostream>
#include <sys/stat.h>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>

#include "ServerSettings.hpp"
#include "HttpRequest.hpp"
#include "HttpAnswer.hpp"
#include "Tools1.hpp"
#include "defines.hpp"

///////////////////////////////////////////////////////////////////////////////]
int Ft_Get::ft_do() {
	printLog(DEBUG, "GET method called", 1);

	if (getCGIStatus() == CGI_DOING)
		return ft_do_cgi();
	else
		return normal_doing();

}

int Ft_Get::normal_doing() {

// is there query in the path? > /script.py?x=abc&y=42
	const HttpRequest& req = getRequest();
	size_t pos = req.getPath().find_first_of('?');
	std::string path;
	std::string query;
	if (pos == std::string::npos)
		path = req.getPath();
	else {
		path = req.getPath().substr(0, pos);
		query = req.getPath().substr(pos + 1);
	}

// add path to root
	std::string ressource; // ressource asked with full path: 'root/ressource - query'
	int rtrn = getFullPath(ressource, path);
	if (rtrn)
		return rtrn;
	oss msg; msg << "Full path of the asked ressource: " << ressource;
	printLog(DEBUG, msg.str(), 1);

// check existance
	struct stat ressource_info;
	rtrn = isFileNOK(ressource, ressource_info);
	if (rtrn) {
		printErr("ressource not OK");
		return rtrn;
	}

// is FILE
	if (S_ISREG(ressource_info.st_mode)) {
		printLog(DEBUG, "is FILE", 1);
		const std::string* CGI_interpreter_path = isCGI(path); // ptr to /usr/bin/python3;
		if (CGI_interpreter_path)
			rtrn = iniCGI(ressource, query, CGI_interpreter_path);
		else
			rtrn = serveFile(ressource, ressource_info);
		return rtrn;
	}
// is DIRECTORY
	else if (S_ISDIR(ressource_info.st_mode)) {
		printLog(DEBUG, "is DIRECTORY", 1);
		if (access(ressource.c_str(), X_OK) != 0) { // even if folder exist, we neeed the rights to traverse it
			printErr(ressource.c_str());
			return 403;
		}
												// Trailing slash edge case : '/dir' != '/dir/' = 301/302? --------------------------------< ???
		std::string ressource_indexed = ressource + *g_settings.find_setting("index");
		struct stat ressource_info2;
		rtrn = isFileNOK(ressource_indexed, ressource_info2);
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
	else
		return 403; // other filesystem objects: symlinks, sockets, devices, FIFOs…
}






///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
static std::string find_MIME_type(const std::string& path);

//-----------------------------------------------------------------------------]
/** Does everything for the GET method. Fills the _answer
 *
 * @return 0 on success, ErrCode on error. Fills answer in case of error	---*/
int Ft_Get::ft_do() {

// is there query in the path? > /script.py?x=abc&y=42
	const HttpRequest& req = getRequest();
	size_t pos = req.getPath().find_first_of('?');
	std::string path;
	std::string query;
	if (pos == std::string::npos)
		path = req.getPath();
	else {
		path = req.getPath().substr(0, pos);
		query = req.getPath().substr(pos + 1);
	}

// add path to root
	std::string ressource;
	int rtrn = getFullPath(ressource, path);
	if (rtrn)
		return rtrn;
	oss msg; msg << "Full path of the asked ressource: " << ressource;
	printLog(DEBUG, msg.str(), 1);

// check existance
	struct stat ressource_info;
	rtrn = isFileNOK(ressource, ressource_info);
	if (rtrn) {
		printErr("ressource not OK");
		return rtrn;
	}

// is FILE
	if (S_ISREG(ressource_info.st_mode)) {
		printLog(DEBUG, "is FILE", 1);
		const std::string* CGI_interpreter_path = isCGI(path);
		if (CGI_interpreter_path)
			rtrn = iniCGI(ressource, query, CGI_interpreter_path);
		else
			rtrn = serveFile(ressource, ressource_info);
		return rtrn;
	}
// is DIRECTORY
	else if (S_ISDIR(ressource_info.st_mode)) {
		printLog(DEBUG, "is DIRECTORY", 1);
		if (access(ressource.c_str(), X_OK) != 0) { // even if folder exist, we neeed the rights to traverse it
			printErr(ressource.c_str());
			return 403;
		}
												// Trailing slash edge case : '/dir' != '/dir/' = 301/302? --------------------------------< ???
		std::string ressource_indexed = ressource + *g_settings.find_setting("index");
		struct stat ressource_info2;
		rtrn = isFileNOK(ressource_indexed, ressource_info2);
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
	else
		return 403; // other filesystem objects: symlinks, sockets, devices, FIFOs…
}

///////////////////////////////////////////////////////////////////////////////]
int Ft_Get::serveFile(const std::string& path, struct stat& ressource_info) {

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


#include <dirent.h>
///////////////////////////////////////////////////////////////////////////////]
/** Generate a listing of requested directory, into _answer
 *
 * 
 *
 * @return      ---*/
int Ft_Get::serveAutoIndexing(const std::string& path) {

	DIR* dir = opendir(path.c_str());
	if (!dir) {
		printErr(path.c_str());
		return 403;
	}

	std::string listing_html = "<html><body><h1>Index of " + getRequest().getPath() + "</h1>";

	struct dirent* entry;
	listing_html += "<ul>";
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;

		// skip "." and maybe ".." if you want
		if (name == ".") continue;
		// if (name == "..") continue;

		std::string base = getRequest().getPath();
		if (base.empty() || base[base.size() - 1] != '/')
			base += '/';
		
		listing_html += "<li><a href=\"" + base + name + "\">" + name + "</a></li>"; // <------------------------------------ ???
	}

	listing_html += "</ul></body></html>";
	getAnswer().setStrBody(listing_html);
	getAnswer().addToHeaders("Content-Type", "text/html");
// Connection: close   (or keep-alive if you support it)
// Optional but recommended:
// Date: <current date>
// Server: Webserv/0.1 (or whatever your server string is)
// Last-Modified (not critical for autoindex)
// Cache-Control: no-cache (optional for dynamic listings)
	closedir(dir);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////]
/**	return MIME type of the file "Content-Type" == text/html 
*
* The headers of the _answer "Content-Type" is filled with this return
*
* @return value of "Content-Type", or "application/octet-stream" in case of error	---*/
static std::string find_MIME_type(const std::string& path) {

	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos)
		return "application/octet-stream";

	const std::string* rtrn = g_settings.find_setting_in_blocks("mime_types", "", path.substr(pos + 1));
	if (!rtrn) {
		std::cerr << RED "unknown MIME type: " RESET << path.substr(pos + 1) << std::endl;
		return "application/octet-stream";
	}
	return *rtrn;
}
