#include "Log.hpp"
#include "Method.hpp"

#include "SettingsServer.hpp"
#include "Tools1.hpp"

#include <sys/stat.h>
#include <cerrno>
#include <stdlib.h>
///////////////////////////////////////////////////////////////////////////////]
/** Check whether a requested resource should be handled as a CGI script
 *
 * Determines if the file extension of the given path matches a CGI handler
 * defined in the server configuration.
 *
 * @param path  Requested resource path (without query string)
 * @return      Pointer to the CGI interpreter path if the resource is CGI,
 *              NULL otherwise			---*/
const std::string* Method::isCGI(const std::string& path) const {

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
int	Method::isFileNOK(std::string path, struct stat& ressource_info) {

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
/** @brief Build the full filesystem path from sanitize path.
 * 
 * Remove the block path and replace with root
 * Ex:
 *   Request path: "/folder/..///./file%201.txt" > "<location_root>/file 1.txt"
 *
 * @param path_to_fill  Reference to a string where the full path will be stored.
 * @return int          0 on success, or 403 if the path attempts to escape the root.	---*/
int Method::getFullPath(std::string& path_to_fill, const std::string& sanitized) const {

	std::vector<std::string> stack;
	std::vector<std::string> v = splitOnDelimitor(sanitized, "/");
// == ["images", "..", ".", "icons", "file.png"]

// remove /location/path from sanitized
	std::vector<std::string> v_location = splitOnDelimitor(_location_block->path, "/");
	v.erase(v.begin(), v.begin() + v_location.size());
// == ["..", ".", "icons", "file.png"]

// check for root escaping
	for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it) {
		if (*it == ".") continue;
		if (*it == "..") {
			if (stack.empty()) {
				LOG_ERROR("Escape attempt from root: " C_431 << sanitized << RESET "]");
				return 403;
			}
			stack.pop_back();
		}
		else
			stack.push_back(*it);
	}

// concatenate back = "/location_folder/file"
	std::string ressource;
	for (std::vector<std::string>::iterator it = stack.begin(); it != stack.end(); ++it) {
		ressource += *it;
		if (it + 1 != stack.end()) ressource += "/";
	}

// add the root of block location_folder = "/given_root/file"
	ressource.insert(0, _location_block->data.root + "/");

	path_to_fill = ressource;
	return 0;
}

// #include <stdlib.h> // strtol()
///////////////////////////////////////////////////////////////////////////////]
/** Decode and validate URL path.
 *  Expands %XX sequences, rejects malformed encodings and control characters.
 *  Example: "/img%2Ftest.png" → "/img/test.png"
 *  @return 0 on success, 400 on invalid path		---*/
int Method::sanitizePath(std::string& path_to_fill, const std::string& given_path) const {

	for (size_t i = 0; i < given_path.size(); ++i) {

		if (given_path[i] == '%') {
			if (i + 2 >= given_path.size()) {
				LOG_ERROR("Truncated %XX sequence");
				return 400;
			}

			char hex[3];
			hex[0] = given_path[i + 1];
			hex[1] = given_path[i + 2];
			hex[2] = '\0';
			char* end = NULL;

			long val = strtol(hex, &end, 16);
			if (*end != '\0') { // invalid hex digits
				LOG_ERROR("Invalid %XX sequence: %" << hex);
				return 400;
			}
			if (val <= 31 || val > 127) { // control chars
				LOG_ERROR("Control / Invalid character in path: %" << hex);
				return 400;
			}
			path_to_fill += static_cast<char>(val);
			i += 2;
		} else {
			char c = given_path[i];
			if ((c >= 0 && c <= 31) || c == 127) { // control chars
				LOG_ERROR("Control character in path");
				return 400;
			}
			path_to_fill += c;
		}
	}
	return 0;
}


///////////////////////////////////////////////////////////////////////////////]
/** Find best matching location block for a request path.
 *  Uses longest-prefix match with boundary checks.
 *  Example: "/folder/sub/file" → matches "location /folder/sub"
 *  @return pointer to matched location_data, or NULL if none		---*/
const block* Method::isLocationKnown(const std::string& given_path) const {

	const std::vector<const block *> locations = g_settings.find_arg_blocks("location");

	const block*	best_match = NULL;
	size_t			best_len   = 0;
	for (size_t i = 0; i < locations.size(); ++i) {
		const block* b = locations[i];
		const std::string& loc = b->path; // e.g. "/images"

		// must be a prefix
		if (given_path.compare(0, loc.size(), loc) != 0) // compare: given_path[0 .. loc.size()-1]  vs  loc
			continue;

		// boundary check: "/img" must not match "/images"
		if (loc != "/" && given_path.size() > loc.size() && given_path[loc.size()] != '/')
			continue;

		// longest match wins
		if (loc.size() > best_len) {
			best_match = b;
			best_len = loc.size();
		}
	}
	return best_match;
}

