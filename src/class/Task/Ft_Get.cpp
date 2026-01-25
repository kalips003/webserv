#include "Ft_Get.hpp"
#include "_colors.h"

#include <iostream>
#include <sys/stat.h>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>

#include "SettingsServer.hpp"
#include "HttpRequest.hpp"
#include "HttpAnswer.hpp"
#include "Tools1.hpp"
#include "Tools2.hpp"
#include "defines.hpp"

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
		if (name == "..") continue;

		std::string base = getRequest().getPath();
		if (base.empty() || base[base.size() - 1] != '/')
			base += '/';
		
		listing_html += "<li><a href=\"" + base + name + "\">" + name + "</a></li>"; // <------------------------------------ ???
	}

	listing_html += "</ul></body></html>";
	getAnswer().setStrBody(listing_html);
	getAnswer().addToHeaders("Content-Type", "text/html");

// Connection: close   (or keep-alive if you support it) // <------------------------------------ ???
// Optional but recommended:
// Date: <current date>
// Server: Webserv/0.1 (or whatever your server string is)
// Last-Modified (not critical for autoindex)
// Cache-Control: no-cache (optional for dynamic listings)
	closedir(dir);
	return 0;
}


