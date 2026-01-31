#include "Log.hpp"
#include "SettingsServer.hpp"

#include <iostream>
#include <algorithm>
#include <sys/stat.h>

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
/** Validate and initialize the server configuration.
 *
 * Ensures minimum settings exist and sets defaults where missing.
 * Validates global parameters like listen port and client_max_body_size.
 * Initializes the root location block and populates all other location blocks
 * with inherited defaults from their ancestor chain.
 *
 * @return	  FALSE on any error, TRUE otherwise				---*/
bool	SettingsServer::check_settings() {
	LOG_LOG("check_settings()");

	default_settings_setup();

	if (!atoi_v2(_global_settings["listen"], _port_num) || 
		_port_num <= 0 || _port_num > 65535) {
			std::cerr << ERR3 "Invalid port number: " << _port_num << std::endl;
			return false;
	}	
	if (!atoi_v2(_global_settings["client_max_body_size"], _client_max_body_size) || _client_max_body_size < -1) {
			std::cerr << ERR3 "Invalid client_max_body_size: " << _client_max_body_size << std::endl;
			return false;
	}
	if (!setRoot())
		return false;

LOG_LOG(C_431 "ROOT BLOCK: \n" RESET << *_root_location_data);

	if (!setTemp())
		return false;

	setAllBlockLocations();

	return true;
}


// #include <algorithm>
///////////////////////////////////////////////////////////////////////////////]
/** Ensure a default "location / {}" block exists and fill missing settings with defaults 
* Also fills _root_location_data		---*/
void	SettingsServer::default_settings_setup() {

	map_strstr default_global_settings;

	default_global_settings["listen"] = "9999";
	default_global_settings["server_name"] = "localhost";
	default_global_settings["error_page"] = "";
	default_global_settings["client_max_body_size"] = "-1";

	// insert skip keys if they already exist
	_global_settings.insert(default_global_settings.begin(), default_global_settings.end());

	block b;
	b.data = location_data();
	b.name = "location";
	b.path = "/";
	b.settings["root"] = "/www";
	b.settings["index"] = "index.html";
	b.settings["autoindex"] = "off";
	b.settings["allowed_methods"] = "GET";
	b.settings["cgi_interpreter"] = "/usr/bin/python3";
	b.settings["client_max_body_size"] = _global_settings.find("client_max_body_size")->second;
	b.hasPath = true;

	// use the operator== (the find block is the first "location / {}")
	std::vector<block>::iterator def = std::find(_block_settings.begin(), _block_settings.end(), b);
	if (def == _block_settings.end()) {
		_block_settings.push_back(b);
		_root_location_data = &(_block_settings.back().data);
	}
	else {
		def->settings.insert(b.settings.begin(), b.settings.end());
		_root_location_data = &((*def).data);
	}
}

// #include <sys/stat.h>
///////////////////////////////////////////////////////////////////////////////]
/** Set the absolute path _root of the server
 *
 * Needs parse_config_file first (and at minimum default root set)
 * remove trailing '/' if present: '/path/' > '/path'
 * @return      True if root exist and is setup, False otherwise		---*/
bool 	SettingsServer::setRoot() {
	LOG_LOG("setRoot()");

	block *root_block = find_root_block();
	std::string root = root_block->settings.find("root")->second;

	if (root.size() > 1 && root[root.size() - 1] == '/')
		root.erase(root.size() - 1);

	if (root[0] == '/') // absolute path
		_root = root;
	else { // relative path, append pwd
		char buf[PATH_MAX];
		if (!getcwd(buf, PATH_MAX))
		{
			LOG_ERROR(ERR8 "getcwd()");
			return false;
		}

		std::string server_path = buf;
		_root = server_path + "/" + root;
	}
// check if root is directory, can be accessed, ...?
	struct stat st;
	if (stat(_root.c_str(), &st) != 0) {
		LOG_ERROR(ERR8 "stat(): Cant access: " << _root);
		return false;
	}

	if (!S_ISDIR(st.st_mode)) {
		LOG_ERROR(ERR9 "stat(): Cant access: " << _root);
		return false;
	}
	root_block->settings["root"] = _root;
	if (!atoi_v2(root_block->settings["client_max_body_size"], root_block->data.client_max_body_size))
		return false;

	std::vector<const block *> empty;
	setLocationData(*root_block, empty);
	return true;
}

///////////////////////////////////////////////////////////////////////////////]
/** Set the absolute path _temp_root of the temp folder
 *
 * Needs parse_config_file first and server _root set up
 * @return      True if temp exist and is setup, False otherwise		---*/
bool 	SettingsServer::setTemp() {
	LOG_LOG("setTemp()");

	const std::string* temp = find_setting("tmp_root");
	std::string temp_folder = temp ? *temp : "/tmp";

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