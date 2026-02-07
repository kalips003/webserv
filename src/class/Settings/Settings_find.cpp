#include "Settings.hpp"

///////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
/** Getter for a _global_settings (name value)
 *
 * @param sett_to_find   Name of the setting to find
 * @param server_block   global block to search
 * @return      Ptr to the string of the setting value, NULL otherwise		---*/
std::string* Settings::find_setting(const std::string& sett_to_find, server_setting& server_block) {

	map_strstr::iterator it = server_block._settings.find(sett_to_find);
	if (it == server_block._settings.end()) {
		LOG_DEBUG(C_511 "setting not found: " RESET << sett_to_find);
		return NULL;
	}
	else 
		return &(it->second);
}

///////////////////////////////////////////////////////////////////////////////]
/** Getter for an arged block (block /path {<block>})
 *
 * @param block_name   Name of the blocks to find
 * @return      Vector of pointer to const block with said name		---*/
std::vector<Settings::block*> Settings::find_arg_blocks(const std::string& block_name, server_setting& a_global_block) {

	std::vector<block*> rtrn;
	for (std::vector<block>::iterator it = a_global_block._blocks.begin(); it != a_global_block._blocks.end(); ++it) {
		if (it->name == block_name && it->hasPath)
			rtrn.push_back(&(*it));
	}
	return rtrn;
}

//-----------------------------------------------------------------------------]
/** Getter for an arged block (block /path {<block>}) const version
 *
 * @param block_name   Name of the blocks to find
 * @return      Vector of pointer to const block with said name		---*/
const std::vector<const Settings::block*> Settings::find_arg_blocks(const std::string& block_name, const server_setting& a_global_block) {

	std::vector<const block*> rtrn;
	for (std::vector<block>::const_iterator it = a_global_block._blocks.begin(); it != a_global_block._blocks.end(); ++it) {
		if (it->name == block_name && it->hasPath)
			rtrn.push_back(&(*it));
	}
	return rtrn;
}


///////////////////////////////////////////////////////////////////////////////]
const Settings::server_setting*		Settings::find_global_block(const std::string& block_name) {

	for (std::map<std::string, Settings::server_setting>::const_iterator it = _global_blocks.begin();
		it != _global_blocks.end(); ++it) {
		
		if (it->first == block_name)
			return &it->second;	
	}
	return NULL;
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
const std::string*	Settings::isCGI(const std::string& path) {

	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos)
		return NULL;
	std::string extension = path.substr(pos);
	
	for (std::map<std::string, Settings::server_setting>::iterator it = _global_blocks.begin(); it != _global_blocks.end(); ++it) {
		if (it->first == "cgi") {
			return Settings::find_setting(extension, it->second);
		}
	}
	return NULL;
}
