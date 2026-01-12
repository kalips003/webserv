#include "ServerSettings.hpp"

#include <iostream>

ServerSettings g_settings;

///////////////////////////////////////////////////////////////////////////////]
/** Getter for a setting
 *
 * @param set   Name of the setting to find
 * @return      String of the setting value, empty string "" otherwise		---*/
std::string ServerSettings::find_setting(const std::string& setting) const {

	map_strstr::const_iterator it = _global_settings.begin();
	it = _global_settings.find(setting);
	if (it == _global_settings.end()) {
		std::cerr << RED "setting not found: " RESET << setting << std::endl;
		return "";
	}
	else 
		return it->second;
}

///////////////////////////////////////////////////////////////////////////////]
/** Getter for a block setting
 *
 * <TODO> empty for now			---*/
std::string ServerSettings::find_setting_inBlock(const std::string& set) const {
	(void)set;
	return "";
}

#include <unistd.h>
#include "Tools1.hpp"
#include <sys/stat.h>
///////////////////////////////////////////////////////////////////////////////]
/** Set the root for the server
 *
 * Needs parse_config_file first
 *
 *
 * @return      True if root exist and is setup, False otherwise		---*/
bool 	ServerSettings::setRoot() {

	std::string root = find_setting("root");
	if (root.size() > 1 && root[root.size() - 1] == '/')
    	root.erase(root.size() - 1);

	if (root[0] == '/') // absolute path
		_root = root;
	else { // relative path, append pwd
		char buf[PATH_MAX];
		if (!getcwd(buf, PATH_MAX))
			return printErr(ERR8 "getcwd()");

		std::string server_path = buf;
		_root = server_path + "/" + root;
	}
// check if root is directory, can be accessed, ...?
	struct stat st;
	if (stat(_root.c_str(), &st) != 0)
		return printErr(ERR8 "stat()");

	if (!S_ISDIR(st.st_mode))
		return printErr(ERR8 "stat()");
	return true;
}
