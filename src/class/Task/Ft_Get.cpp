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

///////////////////////////////////////////////////////////////////////////////]
static int	isFileNOK(std::string path, struct stat& ressource_info);
static std::string find_MIME_type(const std::string& path);

//-----------------------------------------------------------------------------]
/** Does everything for the GET method. Fills the _answer
 *
 * 
 *
 * @return 0 on success, ErrCode on error. Fills answer in case of error	---*/
int Ft_Get::ft_do() {
	std::cout << C_431 "IM ALIVE! (GET)" RESET << std::endl;
// add path to root
	std::string ressource = g_settings.getRoot() + getRequest().getPath(); // /root/index.html

	std::cout << "ressource_path: " << ressource << std::endl;

// check existance
	struct stat ressource_info;
	int rtrn = isFileNOK(ressource, ressource_info);
	if (rtrn) {
		printErr("ressource not OK");
		return rtrn;
	}

	if (S_ISREG(ressource_info.st_mode)) { // if file
	
		if (access(ressource.c_str(), R_OK) != 0) // even if file exist, might not be readable by server
			return 403;

		int fd = open(ressource.c_str(), O_RDONLY);
		if (fd < 0) {
			printErr("open()");
			return 500;
		}
		getAnswer().setFd(fd);
		getAnswer().setBodySize(ressource_info.st_size);
		getAnswer().addToHeaders("Content-Type", find_MIME_type(ressource));
	
	}




// what is it? directory or file?
// if dir, check for default index, else generate listing
	// if (S_ISDIR(ressource_info.st_mode)) { // <------------------------------------------------------------------------????
	// 	std::string defaultt = g_settings.find_setting("index");
	// 	if (!isFileNOK(ressource + defaultt)) {
	// 		std::string autoindex = g_settings.find_setting("autoindex");
	// 		if (autoindex == "on") {
	// 			// generate directory listing
	// 			std::cout << "hello\n";
	// 		}
		
		
	// 	}
	// 	// it’s a directory
	// } else if (S_ISREG(ressource_info.st_mode)) {
	// 			std::cout << "hello2\n";
	// 	// regular file
	// } else {
	// 			std::cout << "hello3\n";
	// 	// maybe a symlink, device, etc
	// 	// usually forbidden in web server
	// }

	// if (access(ressource.c_str(), R_OK) != 0) {
	// 	// not readable → 403
	// }
	// off_t file_size = ressource_info.st_size; // bytes

	return 0;
}

/**	@return 0 if all ok, ErrCode else */
static int	isFileNOK(std::string path, struct stat& ressource_info) {

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

	std::string listing_html = "<html><body><h1>Index of " + getRequest().getPath() + "</h1>";

	struct dirent* entry;
	listing_html += "<ul>";
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;

		// skip "." and maybe ".." if you want
		if (name == ".") continue;
		if (name == "..") continue;

		listing_html += "<li><a href=\"" + name + "\">" + name + "</a></li>";
	}

	listing_html += "</ul></body></html>";
	getAnswer().setStrBody(listing_html);
	closedir(dir);
	return 0; // <------------------------------------------------------------------------????
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

// {
// 	std::map<std::string, std::string> mime_types;

// 	mime_types["html"] = "text/html";
// 	mime_types["css"]  = "text/css";
// 	mime_types["js"]   = "application/javascript";
// 	mime_types["png"]  = "image/png";
// 	mime_types["jpg"]  = "image/jpeg";
// 	mime_types["jpeg"] = "image/jpeg";
// 	mime_types["gif"]  = "image/gif";
// 	mime_types["txt"]  = "text/plain";

// }