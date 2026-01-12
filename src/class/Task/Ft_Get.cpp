#include "Ft_Get.hpp"
#include "_colors.h"

#include <iostream>
#include <sys/stat.h>
#include <cerrno>
#include <unistd.h>

#include "ServerSettings.hpp"
#include "HttpRequest.hpp"
#include "HttpAnswer.hpp"
#include "Tools1.hpp"
///////////////////////////////////////////////////////////////////////////////]
//-----------------------------------------------------------------------------]
int Ft_Get::ft_do() {
	std::cout << C_431 "IM ALIVE! (GET)" RESET << std::endl;
// add path to root
	std::string ressource = g_settings.getRoot() + getRequest().getPath(); // / index.html

	std::cout << "ressource_path: " << ressource << std::endl;

// check existance
	struct stat ressource_info;
	int rtrn = isFileNOK(ressource);
	if (rtrn)
		return rtrn;

// what is it? directory or file?
	if (S_ISDIR(ressource_info.st_mode)) {
		std::string defaultt = g_settings.find_setting("index");
		if (!isFileNOK(ressource + defaultt)) {
			std::string autoindex = g_settings.find_setting("autoindex");
			if (autoindex == "on") {
				// generate directory listing
			
			
			}
		
		
		}
		// it’s a directory
	} else if (S_ISREG(ressource_info.st_mode)) {
		// regular file
	} else {
		// maybe a symlink, device, etc
		// usually forbidden in web server
	}

	if (access(ressource.c_str(), R_OK) != 0) {
		// not readable → 403
	}

	off_t file_size = ressource_info.st_size; // bytes
	return 0;
}

static int	isFileNOK(std::string path) {
	struct stat ressource_info;

	int rtrn = stat(path.c_str(), &ressource_info);
	if (rtrn) {
		if (errno == ENOENT) // ENOENT → file not found → 404
			return 404;
		if (errno == EACCES) // EACCES → permission denied → 403
			return 403;
	}

	return 0;
}

#include <dirent.h>
///////////////////////////////////////////////////////////////////////////////]
/** Generate a listing of requested directory, into _answer
 *
 * 
 *
 * @return      ---*/
int Ft_Get::generate_listing(std::string path) {

	DIR* dir = opendir(path.c_str());
	if (!dir) {
		setStatus(403); // cannot open → forbidden
		return 403;
	}

	std::string listing_html = "<html><body><h1>Index of " + getRequest().getPath() + "</h1><ul>";

	struct dirent* entry;
	while ((entry = readdir(dir)) != nullptr) {
		std::string name = entry->d_name;

		// skip "." and maybe ".." if you want
		if (name == ".") continue;

		listing_html += "<li><a href=\"" + name + "\">" + name + "</a></li>";
	}

	listing_html += "</ul></body></html>";
	getAnswer().setStrBody();
	closedir(dir);


}
