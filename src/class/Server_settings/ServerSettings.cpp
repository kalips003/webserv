#include "ServerSettings.hpp"

#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

#include "Tools1.hpp"

ServerSettings g_settings;

///////////////////////////////////////////////////////////////////////////////]
/** Getter for a _global_settings
 *
 * @param set   Name of the setting to find
 * @return      Ptr to the string of the setting value, NULL otherwise		---*/
const std::string* ServerSettings::find_setting(const std::string& setting) const {

	map_strstr::const_iterator it = _global_settings.begin();
	it = _global_settings.find(setting);
	if (it == _global_settings.end()) {
		oss msg; msg << C_511 "setting not found: " RESET << setting;
		printLog(WARNING, msg.str(), 1);
		return NULL;
	}
	else 
		return &(it->second);
}

///////////////////////////////////////////////////////////////////////////////]
/** Getter for a global block (no arg / path)
 *
 * @param block_name   Name of the global block to find
 * @return      Pointer to const block, NULL if not found		---*/
const block* ServerSettings::find_global_block(const std::string& block_name) const {

	std::vector<block>::const_iterator it = _block_settings.begin();
	for ( ; it != _block_settings.end(); ++it) {
		if (it->name == block_name)
			return &(*it);
	}
	if (it == _block_settings.end()) {
		std::cerr << RED "block not found: " RESET << block_name << std::endl;
		return NULL;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////]
/** Getter for an arged block
 *
 * @param block_name   Name of the blocks to find
 * @return      Vector of pointer to const block with said name		---*/
const std::vector<const block*> ServerSettings::find_arg_blocks(const std::string& block_name) const {

	std::vector<const block*> rtrn;
	for (std::vector<block>::const_iterator it = _block_settings.begin(); it != _block_settings.end(); ++it) {
		if (it->name == block_name && it->hasPath)
			rtrn.push_back(&(*it));
	}
	return rtrn;
}

//-----------------------------------------------------------------------------]
const block*	ServerSettings::find_arg_block_from_vector(const std::vector<const block*>& v, const std::string& arg_name) const {

	for (std::vector<const block*>::const_iterator it = v.begin(); it != v.end(); ++it) {
		if ((*it)->arg == arg_name)
			return *it;
	}
	return NULL;
}


//-----------------------------------------------------------------------------]
const std::string*	ServerSettings::find_setting_in_block(const block* b, const std::string& setting) const {

	if (!b)
		return NULL;
	map_strstr::const_iterator it = b->settings.find(setting);
	if (it == b->settings.end()) {
		oss msg; msg << C_522 "setting not found: " RESET << setting;
		printLog(DEBUG, msg.str(), 1);
		return NULL;
	}
	else 
		return &it->second;
}

///////////////////////////////////////////////////////////////////////////////]
/** Getter for a block setting
 *
 * @return const pointer to the value of setting if found, NULL otherwise			---*/
const std::string* ServerSettings::find_setting_in_blocks(const std::string& block_name, const std::string& arg, const std::string& setting) const {

	const block* b;
	if (arg == "")
		b = find_global_block(block_name);
	else {
		const std::vector<const block*> vect = find_arg_blocks(block_name);
		b = find_arg_block_from_vector(vect, arg);
	}
	return find_setting_in_block(b, setting);
}

// #include <unistd.h>
// #include "Tools1.hpp"
// #include <sys/stat.h>
///////////////////////////////////////////////////////////////////////////////]
/** Set the root for the server
 *
 * Needs parse_config_file first (and at minimum default root set)
 *
 * @return      True if root exist and is setup, False otherwise		---*/
bool 	ServerSettings::setRoot() {

	std::string root(*find_setting("root"));
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
		return printErr(ERR9 "stat()");
	return true;
}

///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, const ServerSettings& setting) {

	os << C_214 "_root: " RESET << setting._root << std::endl;
	os << C_214 "_port_num: " RESET << setting._port_num << std::endl;
	os << C_214 UNDER "\n_global_settings" R_UNDER ":" RESET << std::endl;
	for (map_strstr::const_iterator it = setting._global_settings.begin(); it != setting._global_settings.end(); ++it) {
		os << it->first << ": " << it->second << std::endl;
	}

	os << C_521 UNDER "\n_block_settings" R_UNDER RESET ":\n";
	for (std::vector<block>::const_iterator it = setting._block_settings.begin(); it != setting._block_settings.end(); ++it) {
		os << it->name << ": \"" << it->arg << "\" {" << std::endl;
		for (map_strstr::const_iterator it2 = it->settings.begin(); it2 != it->settings.end(); ++it2) {
			os << it2->first << ": " << it2->second << std::endl;
		}
		os << "}\n" << std::endl;
	}

	return os;
}
