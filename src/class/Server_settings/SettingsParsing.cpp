#include "SettingsServer.hpp"

#include <iostream>
#include <fstream>

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
static bool	parse_key_value(std::string& line, std::pair<std::string, std::string>& rtrn);
static bool	parse_blocks(std::ifstream& file, std::string& line, size_t pos, block& b);
///////////////////////////////////////////////////////////////////////////////]
/** MAIN function parsing the infile.conf
 *
 * DOES NOT check for the validity of the settings 
 *
 * @param confi_file   Valid char* of the path to the config file
 * @return	  FALSE on any parsing error, TRUE otherwise			---*/
bool	SettingsServer::parse_config_file(const char* confi_file) {

	std::string s(confi_file);
	if (s.size() < 5 || s.substr(s.size() - 5) != ".conf") {
		std::cerr << ERR "one .conf arg is required" RESET << std::endl;
		return false;
	}

	std::ifstream   file(confi_file);
	if (!file.is_open()) {
		std::cerr << ERR "cant open file: " << C_420 << confi_file << RESET << std::endl;
		return false;
	}

	std::string line;
	while (std::getline(file, line)) {
		line = trim_white(line);
		if (line.empty() || line[0] == '#')
			continue;
		size_t pos = line.find('{');
		if (pos != std::string::npos) {
			block   b;
			if (!parse_blocks(file, line, pos, b))
				return false;
			addBlock(b);
		}
		else {
			std::pair<std::string, std::string> kv;
			if (!parse_key_value(line, kv))
				return false;
			addSetting(kv.first, kv.second);
		}
	}
		
	return true;
}

///////////////////////////////////////////////////////////////////////////////]
// read: "server_name myserver.local"
// return true if pair is correct
static bool	parse_key_value(std::string& line, std::pair<std::string, std::string>& rtrn) {

	std::stringstream			ss(trim_white(line.substr(0, line.find_first_of("#"))));
	std::vector<std::string>	tokens;
	std::string					tok;

	while (ss >> tok)
		tokens.push_back(tok);

	if (tokens.size() != 2) {
		std::cerr << ERR0 RED "invalid line: [" RESET << line << RED "]" RESET << std::endl;
		return false;
	}
	rtrn = std::make_pair(tokens[0], tokens[1]);

	return true;
}

///////////////////////////////////////////////////////////////////////////////]
static bool	parse_block_key_value(std::string& line, block& b) {

	std::stringstream		 	ss(trim_white(line));
	std::vector<std::string>	tokens;
	std::string				 	tok;

	while (ss >> tok)
		tokens.push_back(tok);

	if (tokens.size() == 2) {
		b.name = tokens[0];
		b.path = tokens[1];
		b.hasPath = true;
	}
	else if (tokens.size() == 1) {
		b.name = tokens[0];
		b.hasPath = false;
	}
	else {
		std::cerr << ERR0 RED "invalid line: [" RESET << line << RED "]" RESET << std::endl;
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////]
// reads: "location /images {
//				  root /var/www/images; hello cat
//				  cgi /usr/bin/python3} a"
static bool	parse_blocks(std::ifstream& file, std::string& line, size_t pos, block& b) {

	std::string s = line.substr(0, pos);

// extract name (& arg)
	if (!parse_block_key_value(s, b)) {
		std::cerr << ERR0 RED "invalid kv value: [" RESET << s << RED "]" RESET << std::endl;
		return false;
	}

// concatenate all block:
// s = "{\nroot /var/www/images; hello cat\n cgi /usr/bin/python3\n}"
	s = line.substr(pos + 1);
	std::string s2;
	while (std::getline(file, s2)) {
		s += "\n" + s2;
		if (s2.find('}') != std::string::npos)
			break;
	}
// replace ';' with '\n'
	for (size_t i = 0; i < s.size(); i++)
		if (s[i] == ';') s[i] = '\n';

// check validity of block, fills settings
	std::stringstream ss(s);
	while (getline(ss, s2)) { // name arg1 arg2 ... [; || \n] delete # comments
		s2 = s2.substr(0, s2.find_first_of("#"));
		s2 = trim_white(s2);
		if (s2.empty()) continue;
		if (s2.find('}') != std::string::npos)
			s2 = trim_white(s2.substr(0, s2.size() - 1));
		if (s2.empty()) return true;
		if (s2.find('}') != std::string::npos) {
			oss msg; msg << ERR0 RED "invalid block: [" RESET << s << RED "]" RESET;
			printLog(ERROR, msg.str(), 1);
			return false;
		}
		size_t pos = s2.find_first_of(' \t');
		if (pos == std::string::npos) {
			oss msg; msg << ERR0 RED "invalid block: [" RESET << s << RED "]" RESET;
			printLog(ERROR, msg.str(), 1);
			return false;
		}
		b.settings[s2.substr(0, pos)] = trim_white(s2.substr(pos + 1));
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
/** Check that the config file has the minimum settings
 , set them to default if missing
 *
 * Also set the _port_num from which the server will listen (default 8080)
 *
 * DOES NOT YET check for the validity of the others settings 
 *
 * <TODO> check if the settings directory can be opened
 * @return	  FALSE on any error, TRUE otherwise				---*/
bool	SettingsServer::check_settings() {

	default_settings_setup();

	// if (); // sort the vector<block> with aglo and operator<>?
	
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

	for (std::vector<block>::iterator it = _block_settings.begin(); it != _block_settings.end(); ++it) {
		if (it->hasPath == false)
			continue;
		map_strstr::iterator it2 = it->settings.find("root");
		if (it2 != it->settings.end()) {
			if (!checkAnyRoot(it2->second))
				return false;
		}
		setLocationData(*it);
	}
	return true;
}


#include <algorithm>
///////////////////////////////////////////////////////////////////////////////]
/** Ensure a default "location / {}" block exists and fill missing settings with defaults 
* Also fills _root_location_data	---*/
void	SettingsServer::default_settings_setup() {

	map_strstr default_global_settings = {
		{"listen", "9999"},
		{"server_name", "localhost"},
		{"error_page", ""},
		{"client_max_body_size", "-1"}
	};
	// insert skip keys if they already exist
	_global_settings.insert(default_global_settings.begin(), default_global_settings.end());

	block b = {
		location_data(),
		"location",
		"/",
		{
			{ "root", "/www" },
			{ "index", "index.html" },
			{ "autoindex", "off" },
			{ "allowed_methods", "GET" },
			{ "cgi_interpreter", "/usr/bin/python3" },
			{ "client_max_body_size", _global_settings.find("client_max_body_size")->second }
		},
		true
	};

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

///////////////////////////////////////////////////////////////////////////////]
/**	Add necessary settings to given block */
bool	SettingsServer::setLocationData(block& b) {

// std::string				root;
	map_strstr::const_iterator it = b.settings.find("root");
	b.data.root = it == b.settings.end() ? _root_location_data->root : it->second;
// std::string				post_policy;
	it = b.settings.find("post_policy");
	b.data.post_policy = it == b.settings.end() ? _root_location_data->post_policy : it->second;
// std::string				cgi_interpreter;
	it = b.settings.find("cgi_interpreter");
	b.data.cgi_interpreter = it == b.settings.end() ? _root_location_data->cgi_interpreter : it->second;
// bool						autoindex;
	it = b.settings.find("autoindex");
	if (it == b.settings.end())
		b.data.autoindex = _root_location_data->autoindex;
	else
		b.data.autoindex = it->second == "on" ? true : false;
// std::vector<std::string>	index;
	it = b.settings.find("index");
	if (it == b.settings.end())
		b.data.index = _root_location_data->index;
	else {
		std::string s;
		std::istringstream iss(it->second);
		while (iss >> s) b.data.index.push_back(s);
	}
// std::vector<std::string>	allowed_methods;
	it = b.settings.find("allowed_methods");
	if (it == b.settings.end())
		b.data.index = _root_location_data->allowed_methods;
	else {
		std::string s;
		std::istringstream iss(it->second);
		while (iss >> s) b.data.allowed_methods.push_back(s);
	}
// ssize_t					client_max_body_size;
	it = b.settings.find("client_max_body_size");
	if (it == b.settings.end())
		b.data.client_max_body_size = _root_location_data->client_max_body_size;
	else {
		if (!atoi_v2(it->second, b.data.client_max_body_size))
			return false;
	}

	return true;
}
