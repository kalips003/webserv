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
