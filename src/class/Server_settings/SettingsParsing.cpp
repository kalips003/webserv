#include "Log.hpp"
#include "SettingsServer.hpp"

#include <iostream>
#include <fstream>

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
static bool	parse_key_value(std::string& line, std::pair<std::string, std::string>& rtrn);
static bool	parse_block_key_value(std::string& line, block& b);
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

//-----------------------------------------------------------------------------]
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

//-----------------------------------------------------------------------------]
/** @return true if a block has correct name (and either a path or no path) */
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

//-----------------------------------------------------------------------------]
/** Parse a single configuration block from the input file.
 *
 * Concatenates lines until the closing '}' is found, replaces ';' with '\n',
 * removes comments, trims whitespace, and fill the given block b.
 *
 * Example:
 *   location /images {
 *       root /var/www/images; hello cat
 *       cgi /usr/bin/python3
 *   }
 *
 * @param file  Input file stream at start of block
 * @param line  Current line containing block header
 * @param pos   Position of '{' in `line`
 * @param b     Block object to populate
 * @return true if parsing succeeds, false on syntax error
 * @note param without values are given an empty string ""	---*/
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
			LOG_ERROR(ERR0 RED "invalid block: [" RESET << s << RED "]" RESET);
			return false;
		}
		size_t pos = s2.find_first_of(" \t");
		if (pos == std::string::npos) {
			LOG_WARNING(ERR0 RED "block are without value: [" RESET << s << RED "]" RESET);
			b.settings[s2] = "";
			continue ;
		}
		b.settings[s2.substr(0, pos)] = trim_white(s2.substr(pos + 1));
	}

	return true;
}
