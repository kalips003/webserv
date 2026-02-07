#include "Method.hpp"

#include "Settings.hpp"

///////////////////////////////////////////////////////////////////////////////]
/** Common handling of a request for normal (non-CGI) processing.
 * - Extracts query string from the path (/script.py - ?x=abc&y=42)
 * - Resolves full filesystem path (/root/_path)
 * - Checks existence and type of the resource
 * - Dispatches to Derived Class the handling of CGI, file, or directory as appropriate
 *
 * @return Connection::SENDING on success, or HTTP error code on failure	---*/
int Method::normal_doing() {

// is there query in the path? > /script.py?x=abc&y=42
	std::string path;
	std::string query;
	size_t pos = _request.getPath().find_first_of('?');
	if (pos == std::string::npos)
		path = _request.getPath();
	else {
		path = _request.getPath().substr(0, pos);
		query = _request.getPath().substr(pos + 1);
	}

// sanitize given_path for %XX;
	std::string sanitized;
	if (Settings::sanitizePath(sanitized, path))
		return 400;
	LOG_LOG("path (" <<  _request.getPath() << ") after sanitizePath: " << sanitized);

// set Method::_location_data* from all the location /blocks
	_location_block = Settings::isLocationKnown(sanitized, *_data._settings); // validity checked in request.validateLocationBlock()

// add root to path (- location_path)
	std::string ressource; // ressource asked with absolute path: 'root/ressource - query'
	Settings::getFullPath(ressource, sanitized, *_data._settings); // validity checked in request.validateLocationBlock()
	LOG_LOG(_request.getMethod() << " ) Full path of the asked ressource: " << ressource);

// if no trailing / in directory path, redirect
	struct stat ressource_info;
	stat(ressource.c_str(), &ressource_info);
	if (S_ISDIR(ressource_info.st_mode) && sanitized[sanitized.size() - 1] != '/') {
		_answer.setFirstLine(301);
		_answer.addToHeaders("Location", sanitized + "/");
		return Connection::SENDING;
	}

// check "Content-Type" = "multipart/form-data";
	int rtrn = this->treatContentType(ressource, query);
	if (rtrn != -1)
		return rtrn;

// HANDLE FILE
	return handleRessource(ressource, query);
}

///////////////////////////////////////////////////////////////////////////////]
/**	handle either
// 	file not exist
// 	file exist
//		directory ---*/
int Method::handleRessource(std::string& ressource, std::string& query) {

// DOESNT EXIST
	int rtrn;
	struct stat ressource_info;
	rtrn = isFileNOK(ressource, ressource_info);
	if (rtrn)
		return this->howToHandleFileNotExist(ressource, rtrn);

// is FILE
	if (S_ISREG(ressource_info.st_mode)) {
		LOG_DEBUG("is FILE");
		const std::string* CGI_interpreter_path = g_settings.isCGI(ressource); // ptr to /usr/bin/python3;
		if (CGI_interpreter_path)
			return iniCGI(ressource, query, CGI_interpreter_path);
		else
			return this->handleFileExist(ressource);
	}
// is DIRECTORY 
	else if (S_ISDIR(ressource_info.st_mode)) {
		LOG_DEBUG("is DIRECTORY");
		return this->handleDir(ressource);
	}
	else
		return 403; // other filesystem objects: symlinks, sockets, devices, FIFOs…
}
