#include "Log.hpp"
#include "SettingsServer.hpp"

#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
/** Normalize and validate a server root path.
 *
 * - `some_root` can be relative (appended to `_root`) or absolute.
 * - Removes trailing '/' (except for root '/') and ensures the path exists.
 * - Must be a directory. Updates `some_root` in-place.
 *
 * @param some_root  Path to normalize and validate.
 * @return true if valid; false otherwise.	---*/
bool 	SettingsServer::checkAnyRoot(std::string& some_root) {
	LOG_LOG("checkAnyRoot(): some_root=\"" << some_root << "\"");

	std::string root = some_root;

	if (root.size() > 1 && root[root.size() - 1] == '/')
		root.erase(root.size() - 1);

	if (root[0] != '/') // relative path, append server_root
		root = _root + '/' + root;

// check if root is directory, can be accessed?
	struct stat st;
	if (stat(root.c_str(), &st) != 0) {
		LOG_ERROR("Issue with some config root: " << root);
		LOG_ERROR(ERR8 "stat()");
		return false;
	}

	if (!S_ISDIR(st.st_mode)) {
		LOG_ERROR("Issue with some config root (not a directory): " << root);
		LOG_ERROR(ERR9 "stat()");
		return false;
	}

	some_root = root;
	return true;
}

///////////////////////////////////////////////////////////////////////////////]
//	SERVER SETTINGS
std::ostream& operator<<(std::ostream& os, const SettingsServer& setting) {

	os << C_214 "_root: " RESET << setting._root << std::endl;
	os << C_214 "_port_num: " RESET << setting._port_num << std::endl;
	os << C_214 UNDER "\n_global_settings" R_UNDER ":" RESET << std::endl;
	for (map_strstr::const_iterator it = setting._global_settings.begin(); it != setting._global_settings.end(); ++it) {
		os << it->first << ": " << it->second << std::endl;
	}

	os << C_521 UNDER "\n_block_settings" R_UNDER RESET ":\n";
	for (std::vector<block>::const_iterator it = setting._block_settings.begin(); it != setting._block_settings.end(); ++it) {
		os << it->name << ": \"" << it->path << "\" {" << std::endl;
		for (map_strstr::const_iterator it2 = it->settings.begin(); it2 != it->settings.end(); ++it2) {
			os << it2->first << ": " << it2->second << std::endl;
		}
		os << "}\n" << std::endl;
	}

	return os;
}


///////////////////////////////////////////////////////////////////////////////]
//	LOCATION_DATA
std::ostream& operator<<(std::ostream& os, const location_data& b) {
	os << C_045 "location_data {" RESET << std::endl;

	os << "  root: " << b.root << std::endl;
	os << "  index: [";
	for (size_t i = 0; i < b.index.size(); ++i) {
		os << b.index[i];
		if (i + 1 != b.index.size()) os << ", ";
	}
	os << "]" << std::endl;

	os << "  autoindex: " << (b.autoindex ? "true" : "false") << std::endl;
	os << "  post_policy: " << b.post_policy << std::endl;
	os << "  cgi_interpreter: " << b.cgi_interpreter << std::endl;

	os << "  allowed_methods: [";
	for (size_t i = 0; i < b.allowed_methods.size(); ++i) {
		os << b.allowed_methods[i];
		if (i + 1 != b.allowed_methods.size()) os << ", ";
	}
	os << "]" << std::endl;

	os << "  client_max_body_size: " << b.client_max_body_size << std::endl;

	os << C_045 "}" RESET << std::endl;
	return os;
}

///////////////////////////////////////////////////////////////////////////////]
//	BLOCK
std::ostream& operator<<(std::ostream& os, const block& b) {
	os << "block {" << std::endl;

	os << C_431 << b.name << C_410 " " << b.path << RESET " {" << std::endl;
	for (map_strstr::const_iterator it = b.settings.begin(); it != b.settings.end(); ++it)
		os << C_512 << it->first <<  RESET ": " << it->second << std::endl;
	os << "}" << std::endl;

	os << "DATA: \n" << b.data << std::endl; // requires operator<< for location_data

	return os;
}
