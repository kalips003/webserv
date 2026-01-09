#include "ServerSettings.hpp"

#include <iostream>
#include <fstream>

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
static bool    parse_key_value(std::string& line, std::pair<std::string, std::string>& rtrn);
static bool    parse_blocks(std::ifstream& file, std::string& line, size_t pos, block& b);
///////////////////////////////////////////////////////////////////////////////]
/** MAIN function parsing the infile.conf
 *
 * DOES NOT check for the validity of the settings 
 *
 * @param confi_file   Valid char* of the path to the config file
 * @return      FALSE on any parsing error, TRUE otherwise			---*/
bool	ServerSettings::parse_config_file(const char* confi_file) {

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
static bool    parse_key_value(std::string& line, std::pair<std::string, std::string>& rtrn) {

    std::stringstream           ss(trim_white(line));
    std::vector<std::string>    tokens;
    std::string                 tok;

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
// reads: "location /images {
//                  root /var/www/images; hello cat
//                  cgi /usr/bin/python3} a"
static bool    parse_blocks(std::ifstream& file, std::string& line, size_t pos, block& b) {

    std::string s = line.substr(0, pos);

// extract name & arg
    std::pair<std::string, std::string> kv;
    if (!parse_key_value(s, kv)) {
        std::cerr << ERR0 RED "invalid kv value: [" RESET << s << RED "]" RESET << std::endl;
        return false;
    }
    b.name = kv.first;
    b.arg = kv.second;

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
    while (getline(ss, s2)) {
        s2 = trim_white(s2);
        if (s2.empty()) continue;
        if (s2.find('}') != std::string::npos)
            s2 = trim_white(s2.substr(0, s2.size() - 1));
        if (s2.empty()) return true;
        if (s2.find('}') != std::string::npos || !parse_key_value(s2, kv)) {
            std::cerr << ERR0 RED "invalid block: [" RESET << s << RED "]" RESET << std::endl;
            return false;
        }
        b.settings[kv.first] = kv.second;
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
 * @return      FALSE on any error, TRUE otherwise				---*/
bool	ServerSettings::check_settings() {

	map_strstr defaults;
	defaults["listen"]		= "8080";
	defaults["server_name"] = "myserver.local";
	defaults["root"]		= "/var/www/html";
	defaults["index"]		= "index.html";

	for (map_strstr::iterator it = defaults.begin(); it != defaults.end(); ++it) {
		if (_global_settings.find(it->first) == _global_settings.end()) {
			std::cerr << RED "Necessary setting (" RESET << it->first << RED ") missing from config" RESET << std::endl;
			std::cerr << it->first << C_142 ": set to default (" RESET << it->second << C_142 ")" RESET << std::endl;
			_global_settings[it->first] = it->second;
		}
	}
	if (!atoi_v2(_global_settings["listen"], _port_num) || 
		_port_num <= 0 || _port_num > 65535) {
			std::cerr << ERR3 "Invalid port number: " << _port_num << std::endl;
			return false;
	}
/* CHECK IF WE CAN OPEN THE SETTING DIRECTORIES */
    return true;
}