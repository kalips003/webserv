#include "Task.hpp"

#include "Connection.hpp"
#include "Server.hpp"

#include "Tools1.hpp"
#include "Ft_Get.hpp"
#include "Ft_Post.hpp"
#include "Ft_Delete.hpp"

#include <sys/stat.h>
#include <stdlib.h>
///////////////////////////////////////////////////////////////////////////////]
Task::Task(Connection& connec)
 : _request(connec.getRequest()), _answer(connec.getAnswer()), _status(0), _cgi_status(CGI_NONE) {}

///////////////////////////////////////////////////////////////////////////////]
Task* Task::createTask(const std::string& method, Connection& connec) {
	if (method == "GET")
		return new Ft_Get(connec);
	else if (method == "POST")
		return new Ft_Post(connec);
	else if (method == "PUT")
		return new Ft_Delete(connec);
	// ... other methods
	else
		return NULL;  // unknown method → 405 or reject
}

///////////////////////////////////////////////////////////////////////////////]
/** Check whether a requested resource should be handled as a CGI script
 *
 * Determines if the file extension of the given path matches a CGI handler
 * defined in the server configuration.
 *
 * @param path  Requested resource path (without query string)
 * @return      Pointer to the CGI interpreter path if the resource is CGI,
 *              NULL otherwise			---*/
const std::string* Task::isCGI(const std::string& path) const {

	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos)
		return NULL;
	std::string extension = path.substr(pos);
	return g_settings.find_setting_in_blocks("cgi", "", extension);
}

///////////////////////////////////////////////////////////////////////////////]
/**	Fills the stat struct of the path
*
* @return 0 if all ok, ErrCode else (403 / 404)		*/
int	Task::isFileNOK(std::string path, struct stat& ressource_info) {

	int rtrn = stat(path.c_str(), &ressource_info);
	if (rtrn) {
		if (errno == ENOENT) // ENOENT → file not found → 404
			return 404;
		if (errno == EACCES) // EACCES → permission denied → 403
			return 403;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////]
/** @brief Normalize and sanitize the requested path and build the full filesystem path.
 *
 * Ex:
 *   Request path: "/folder/..///./file%201.txt" > "<server_root>/file 1.txt"
 *
 * @param path_to_fill  Reference to a string where the full path will be stored.
 * @return int          0 on success, or 403 if the path attempts to escape the root.
 *
 * @note The server root returned by g_settings.getRoot() should NOT end with a '/'		---*/
int Task::getFullPath(std::string& path_to_fill, const std::string& given_path) const {

// sanitize given_path for %XX;
	std::string sanitized;
	if (sanitizePath(sanitized, given_path))
		return 400;

	std::vector<std::string> stack;
	std::vector<std::string> v = splitOnDelimitor(sanitized, "/");

	for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it) {
		if (*it == ".") continue;
		if (*it == "..") {
			if (stack.empty()) {
				oss msg; msg << "Escape attempt from root: " C_431 << sanitized << RESET "]";
				printLog(ERROR, msg.str(), 1);
				return 403;
			}
			stack.pop_back();
		}
		else
			stack.push_back(*it);
	}

	std::string ressource;
	for (std::vector<std::string>::iterator it = stack.begin(); it != stack.end(); ++it) {
		ressource += *it;
		if (it + 1 != stack.end()) ressource += "/";
	}
	ressource.insert(0, g_settings.getRoot() + "/");
	path_to_fill = ressource;

	return 0;
}

// #include <stdlib.h> // strtol()
///////////////////////////////////////////////////////////////////////////////]
int Task::sanitizePath(std::string& path_to_fill, const std::string& given_path) const {

	for (size_t i = 0; i < given_path.size(); ++i) {

		if (given_path[i] == '%') {
			if (i + 2 >= given_path.size()) {
				printLog(ERROR, "Truncated %XX sequence", 1);
				return 400;
			}

			char hex[3];
			hex[0] = given_path[i + 1];
			hex[1] = given_path[i + 2];
			hex[2] = '\0';
			char* end = NULL;

			long val = strtol(hex, &end, 16);
			if (*end != '\0') { // invalid hex digits
				oss msg; msg << "Invalid %XX sequence: %" << hex;
				printLog(ERROR, msg.str(), 1);
				return 400;
			}
			if (val <= 31 || val > 127) { // control chars
				oss msg; msg << "Control / Invalid character in path: %" << hex;
				printLog(ERROR, msg.str(), 1);
				return 400;
			}
			path_to_fill += static_cast<char>(val);
			i += 2;
		} else {
			char c = given_path[i];
			if ((c >= 0 && c <= 31) || c == 127) { // control chars
				printLog(ERROR, "Control character in path", 1);
				return 400;
			}
			path_to_fill += c;
		}
	}
	return 0;
}
