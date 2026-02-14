#include "Settings.hpp"

#include <sys/stat.h>
#include <algorithm>

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
bool			default_settings_setup(Settings::server_setting& global_block);
static void		setAllBlockLocations(Settings::server_setting& a_global_block);
static bool 	compareByPathLen(const Settings::block* a, const Settings::block* b);
static bool		setLocationData(Settings::block& for_this_block, const std::vector<const Settings::block*>& map_of_matches, Settings::server_setting& a_global_block);
static std::string rtrnWordMatch(Settings::block& for_this_block, const std::string& setting_to_find, const std::vector<const Settings::block*>& map_of_matches);
static bool 	checkAnyRoot(std::string& some_root, std::string& server_root);
///////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
/** set absolut root of Settings */
bool 	Settings::setRoot() {
// LOG_LOG("setRoot()");

	char buf[PATH_MAX];
	if (!getcwd(buf, PATH_MAX)) {
		LOG_ERROR_SYS(ERR8 "setRoot(): getcwd()");
		return false;
	}
	_root = buf;

// check if root can be accessed
	struct stat st;
	if (stat(_root.c_str(), &st) != 0) {
		LOG_ERROR_SYS(ERR8 "setRoot(): stat(): Cant access: " << _root);
		return false;
	}

	LOG_LOG( GREEN "_root set up = " RESET << _root)
	return true;
}

///////////////////////////////////////////////////////////////////////////////]
/** Set the absolute path _temp_root of the temp folder
 *
 * Needs parse_config_file first and server _root set up
 * @return      True if temp exist and is setup, False otherwise		---*/
bool 	Settings::setTemp() {
// LOG_LOG("setTemp()");

	std::string temp_folder = "/tmp";

	map_strstr::iterator it = _global_settings.find("tmp_root");
	if (it != _global_settings.end())
		temp_folder = it->second;

	if (temp_folder.size() > 1 && temp_folder[temp_folder.size() - 1] == '/')
		temp_folder.erase(temp_folder.size() - 1);

	if (temp_folder[0] != '/')
		_temp_root = _root + "/" + temp_folder;
	else
		_temp_root = temp_folder;

	struct stat st;
//	Check accessibility for the server process
	if (stat(_temp_root.c_str(), &st) != 0) {
		LOG_ERROR(ERR8 "stat(): Can't access: " << _temp_root);
		return false;
	}

//	Check if its a DIR
	if (!S_ISDIR(st.st_mode)) {
		LOG_ERROR(ERR9 << _temp_root << " Isn't a Directory");
		return false;
	}

//	Check permission for the server process
	if (access(_temp_root.c_str(), R_OK | W_OK | X_OK) != 0) {
		LOG_ERROR(ERR7 "Temp folder not accessible (r/w/x): " << _temp_root);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////]
/** at this point, all global blocks have their _settings and _blocks filled
* blocks dont have their data set
* server root is setup 
* the server block has the default "location /" initialized */
bool	Settings::blockSetup(Settings::server_setting& a_global_block, std::string& root) {

// _server_name
	std::string* name = Settings::find_setting("server_name", a_global_block);
	if (!name)
		return true;
	a_global_block._server_name = *name;

// _root
	if (!Settings::find_setting("root", a_global_block))
		a_global_block._settings["root"] = "www"; // default
	a_global_block._root = *Settings::find_setting("root", a_global_block);
	if (!checkAnyRoot(a_global_block._root, root))
		return false;

	if (!default_settings_setup(a_global_block))
		return false;

// _blocks.data
	setAllBlockLocations(a_global_block);
	return true;
}

//-----------------------------------------------------------------------------]
/** Ensure a default "location / {}" block exists and fill missing settings with defaults  ---*/
bool		default_settings_setup(Settings::server_setting& global_block) {

	map_strstr default_global_settings;
// default
	default_global_settings["listen"] = ""; // default
	default_global_settings["client_max_body_size"] = "-1"; // default
	// insert skip keys if they already exist
	global_block._settings.insert(default_global_settings.begin(), default_global_settings.end());

// _port
	std::string* port = Settings::find_setting("listen", global_block);
	if (!atoi_v2(*port, global_block._port) || global_block._port <= 0 || global_block._port > 65535) {
		LOG_ERROR("Invalid port number: " << global_block._port)
		return false;
	}
// _client_max_body_size
	std::string* body_size = Settings::find_setting("client_max_body_size", global_block);
	if (!atoi_v2(*body_size, global_block._client_max_body_size) || global_block._client_max_body_size < -1) {
		LOG_ERROR("Invalid client_max_body_size: " << global_block._client_max_body_size)
		return false;
	}

	Settings::block b;
	b.data = Settings::location_data();
	b.name = "location";
	b.path = "/";
	b.settings["root"] = global_block._root;
	b.settings["index"] = "";
	b.settings["autoindex"] = "off";
	b.settings["allowed_methods"] = "";
	b.settings["cgi_interpreter"] = "";
	oss num; num << global_block._client_max_body_size;
	b.settings["client_max_body_size"] = num.str();
	b.hasPath = true;

	// use the operator== (the find block is the first "location / {}")
	std::vector<Settings::block>::iterator def = std::find(global_block._blocks.begin(), global_block._blocks.end(), b);
	if (def == global_block._blocks.end()) {
		global_block._blocks.push_back(b);
		global_block._root_location_data = &(global_block._blocks.back().data);
	}
	else {
		def->settings.insert(b.settings.begin(), b.settings.end());
		global_block._root_location_data = &((*def).data);
	}
	return true;
}

//-----------------------------------------------------------------------------]
/** Initialize the internal data struct of all blocks that have a path.
 *
 * Invalid or unresolvable blocks are removed from _block_settings, with warnings logged.
 * @note Root '/' block is treated as having its root already setup.	---*/
static void	setAllBlockLocations(Settings::server_setting& a_global_block) {
	LOG_LOG("setAllBlockLocations()");

	for (std::vector<Settings::block>::iterator it = a_global_block._blocks.begin(); it != a_global_block._blocks.end(); ++it) {
		if (!it->hasPath)
			continue;

		if (it->path[0] != '/') {
			LOG_ERROR("Location block without '/': " << it->path);
			it = a_global_block._blocks.erase(it);
			continue ;
		}

		const std::vector<const Settings::block*> parent_tree = Settings::rtrnMapOfMatches(*it, a_global_block);
		if (!setLocationData(*it, parent_tree, a_global_block)) {
			LOG_WARNING(C_431 "Invalid location block detected, ignored:" RESET << *it);
			it = a_global_block._blocks.erase(it);
			continue;
		}
		LOG_DEBUG(C_431 "BLOCK: \n" RESET << *it);
	}
}

//-----------------------------------------------------------------------------]
/** Build an ordered list of ancestor location blocks for a given block.
 *
 * The returned vector is sorted from most specific (longest path) to least specific (/ root last).
 * Does not include the block itself.
 *
 * @param for_this_block  The block whose ancestors are being resolved
 * @return vector of ancestor blocks in descending specificity order	---*/
const std::vector<const Settings::block*>	Settings::rtrnMapOfMatches(Settings::block& for_this_block, Settings::server_setting& a_global_block) {

	std::vector<Settings::block*> locations = Settings::find_arg_blocks("location", a_global_block);
	std::string& given_path = for_this_block.path;
	std::vector<const Settings::block*>	v_rtrn;

	for (size_t i = 0; i < locations.size(); ++i) {

		Settings::block* b = locations[i];
		const std::string& loc = b->path; // e.g. "/images"

		if (b == &for_this_block)
			continue;
		// must be a prefix
		if (given_path.compare(0, loc.size(), loc) != 0) // compare: given_path[0 .. loc.size()-1]  vs  loc
			continue;

		// boundary check: "/img" must not match "/images"
		if (loc != "/" && given_path.size() > loc.size() && given_path[loc.size()] != '/')
			continue;

		v_rtrn.push_back(b);
	}

	std::sort(v_rtrn.begin(), v_rtrn.end(), compareByPathLen);
	return v_rtrn;
}

//-----------------------------------------------------------------------------]
/**	Helper tool for sorting a std::vector<const block*>	---*/
static bool compareByPathLen(const Settings::block* a, const Settings::block* b) {
	return a->path.size() > b->path.size();
}

//-----------------------------------------------------------------------------]
/**  Populate a block's data fields using its ancestor locations.
 * Each setting is taken from the closest ancestor that defines it,
 * with the root location as a guaranteed fallback.
 * 
 * @return true if all settings were successfully set, false on error		
 * @note the root setting is checked further for validity and syntax/ ---*/
static bool	setLocationData(Settings::block& for_this_block, const std::vector<const Settings::block*>& map_of_matches, Settings::server_setting& a_global_block) {

	std::string setting_match;

// std::string				root;
	for_this_block.data.root = rtrnWordMatch(for_this_block, "root", map_of_matches);
	if (!checkAnyRoot(for_this_block.data.root, a_global_block._root))
		return false;

// std::string				post_policy;
	for_this_block.data.post_policy = rtrnWordMatch(for_this_block, "post_policy", map_of_matches);

// std::string				cgi_interpreter;
	for_this_block.data.cgi_interpreter = rtrnWordMatch(for_this_block, "cgi_interpreter", map_of_matches);

// bool						autoindex;
	setting_match = rtrnWordMatch(for_this_block, "autoindex", map_of_matches);
	for_this_block.data.autoindex = (setting_match == "on");

// std::vector<std::string>	index;
	setting_match = rtrnWordMatch(for_this_block, "index", map_of_matches);
	std::string s;
	std::istringstream iss(setting_match);
	while (iss >> s) for_this_block.data.index.push_back(s);

// std::vector<std::string>	allowed_methods;
	setting_match = rtrnWordMatch(for_this_block, "allowed_methods", map_of_matches);
	s = "";
	iss.clear();
	iss.str(setting_match);
	while (iss >> s) for_this_block.data.allowed_methods.push_back(s);

// ssize_t					client_max_body_size;
	setting_match = rtrnWordMatch(for_this_block, "client_max_body_size", map_of_matches);
	if (!atoi_v2(setting_match, for_this_block.data.client_max_body_size))
		for_this_block.data.client_max_body_size = a_global_block._client_max_body_size;

	return true;
}


//-----------------------------------------------------------------------------]
/** Return the first value of a setting found in the ancestor blocks, if not found first in the given block, or "" if none */
static std::string rtrnWordMatch(Settings::block& for_this_block, const std::string& setting_to_find, const std::vector<const Settings::block*>& map_of_matches) {

	map_strstr::const_iterator match = for_this_block.settings.find(setting_to_find);
	if (match != for_this_block.settings.end()) //  && !match->second.empty() if i want inheritance in case of empty
		return match->second;

	for (std::vector<const Settings::block*>::const_iterator it = map_of_matches.begin(); it != map_of_matches.end(); ++it) {
		match = (*it)->settings.find(setting_to_find);
		if (match == (*it)->settings.end())
			continue ;
		return match->second;
	}
	return "";
}

//-----------------------------------------------------------------------------]
/** Normalize and validate a server root path.
 *
 * - `some_root` can be relative (appended to `_root`) or absolute.
 * - Removes trailing '/' (except for root '/') and ensures the path exists.
 * - Must be a directory. Updates `some_root` in-place.
 *
 * @param some_root  Path to normalize and validate.
 * @return true if valid; false otherwise.	---*/
static bool 	checkAnyRoot(std::string& some_root, std::string& server_root) {
	LOG_LOG("checkAnyRoot(): some_root=\"" << some_root << "\"");

	std::string root = some_root;

	if (root.size() > 1 && root[root.size() - 1] == '/')
		root.erase(root.size() - 1);

	if (root[0] != '/') // relative path, append server_root
		root = server_root + '/' + root;

// check if root is directory, can be accessed?
	struct stat st;
	if (stat(root.c_str(), &st) != 0) {
		LOG_ERROR_SYS("Issue with some config root: " << root);
		return false;
	}

	if (!S_ISDIR(st.st_mode)) {
		LOG_ERROR("Issue with some config root (not a directory): " << root);
		return false;
	}

	some_root = root;
	return true;
}