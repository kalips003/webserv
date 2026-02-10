#include "Settings.hpp"

#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
static bool	parse_global_block(std::ifstream& file, std::string& line, std::map<std::string, Settings::server_setting>& global_blocks, int i, std::string& root);
static bool	count_brackets(std::string& line, int& brace_depth);
static v_str split_line_in_tokens(std::string& line);
static bool	parse_tokens(v_str& tokens, Settings::server_setting& server_block, int i);
static void	push_new_directive(v_str& temp, map_strstr& settings);
static bool	push_new_block(v_str& temp, std::vector<Settings::block>& settings, v_str& tokens, v_str::iterator& it_tokens, int i);
///////////////////////////////////////////////////////////////////////////////]
Settings g_settings;
///////////////////////////////////////////////////////////////////////////////]
/** MAIN function parsing the infile.conf
 *
 * Parse the .conf file annd fills the class instance 
 *
 * @param confi_file   Valid char* of the path to the config file
 * @return	  FALSE on any parsing error, TRUE otherwise			
 * @note Does NOT do any Setup								---*/
bool	Settings::parse_config_file(const char* confi_file) {
// LOG_LOG("parse_config_file()")

	if (!setRoot())
		return false;

	std::string s(confi_file);
	if (s.size() < 5 || s.substr(s.size() - 5) != ".conf") {
		LOG_ERROR("one .conf arg is required" RESET)
		return false;
	}

	std::ifstream   file(confi_file);
	if (!file.is_open()) {
		LOG_ERROR("cant open file: " << C_420 << confi_file << RESET)
		return false;
	}

	std::string line;
	int i = 0;
	while (std::getline(file, line) && ++i) {

		line = trim_white(line.substr(0, line.find_first_of("#")));
		if (line.empty())
			continue;
		
		size_t pos = line.find('{');
	// parse for global setting
		if (pos == std::string::npos) {
			v_str	tokens = split_line_in_tokens(line);
			push_new_directive(tokens, _global_settings);
		}
	// parse for global block
		else {
			line.push_back(';');
			if (!parse_global_block(file, line, _global_blocks, i, _root)) // anything after the global block '}' is ignored
				return false;
		}
	}

	if (!setTemp())
		return false;

	return true;
}

#include <algorithm>
///////////////////////////////////////////////////////////////////////////////]
/** parse an entire global block */
static bool	parse_global_block(std::ifstream& file, std::string& line, std::map<std::string, Settings::server_setting>& global_blocks, int i, std::string& root) {
// LOG_LOG("parse_global_block()")

	v_str	tokens = split_line_in_tokens(line);
	v_str::iterator it = std::find(tokens.begin(), tokens.end(), "{");

// validate first line: <global block name> {...
	v_str temp(tokens.begin(), it);
	if (temp.size() != 1) {
		oss err;
		for (v_str::iterator itt = tokens.begin(); itt != it; ++itt)
			err << *itt << " ";
		LOG_ERROR("invalid token number in block designation: " << RED << err.str() << RESET "{ ...")
		return false;
	}
	Settings::server_setting		server_block;
	server_block._server_block_name = temp[0];


	int	brace_depth = 0;
	count_brackets(line, brace_depth);
// read and create tokens_vector until closing bracket } of global block found
	while (brace_depth && std::getline(file, line) && ++i) {

		line = trim_white(line.substr(0, line.find_first_of("#")));
		if (line.empty())
			continue;
		line.push_back(';');

		count_brackets(line, brace_depth);

		v_str new_tokens = split_line_in_tokens(line);
		tokens.insert(tokens.end(), new_tokens.begin(), new_tokens.end());
	}

	// for (v_str::iterator it = tokens.begin(); it != tokens.end(); ++it) {
	// 	std::cout << *it << RED " | " RESET;
	// } 
	if (brace_depth)
		return false; // non matching brackets, fatal

	if (!parse_tokens(tokens, server_block, i))
		return false;
	// is there stuff after the last }, ignored

	if (!Settings::blockSetup(server_block, root))
		return true; // bad block discarded

	if (server_block._server_block_name == "server")
		global_blocks.insert(std::pair<std::string, Settings::server_setting>(server_block._server_name, server_block));
	else
		global_blocks.insert(std::pair<std::string, Settings::server_setting>(server_block._server_block_name, server_block));

	return true;
}

//-----------------------------------------------------------------------------]
static bool	count_brackets(std::string& line, int& brace_depth) {
// LOG_LOG("count_brackets()")

	for (size_t i = 0; i < line.size(); ++i) {
		if (line[i] == '{')
			brace_depth++;
		else if (line[i] == '}')
			brace_depth--;	
	}
	return !brace_depth;
}

//-----------------------------------------------------------------------------]
static v_str split_line_in_tokens(std::string& line) {
// LOG_LOG("split_line_in_tokens():")

	v_str rtrn;

	std::string word;
	for (size_t i = 0; i < line.size(); ++i) {
		if (wii(line[i], " \r\n\t") != -1) {
			if (word.size()) {
				rtrn.push_back(word);
				word.clear();
			}
			continue;
		}
		if (wii(line[i], ";{}") != -1) {
			if (word.size()) {
				rtrn.push_back(word);
				word.clear();
			}
			rtrn.push_back((std::string(1, line[i])));
		}
		else
			word.push_back(line[i]);
	}

	if (!word.empty())
		rtrn.push_back(word);

	return rtrn;
}

#include <algorithm>
///////////////////////////////////////////////////////////////////////////////]
static bool	parse_tokens(v_str& tokens, Settings::server_setting& server_block, int i) {
// LOG_LOG("parse_tokens()")

	v_str::iterator it = std::find(tokens.begin(), tokens.end(), "{");
	++it;

	v_str temp;
	for ( ; it != tokens.end(); ++it) {
		temp.clear();

		// collect tokens until one of the special tokens
		for ( ; it != tokens.end() && *it != ";" && *it != "{" && *it != "}"; ++it)
			temp.push_back(*it);

		if (it == tokens.end()) // < if temp not empty, there is stuff after the last '}'
			break;
		
	// temp contains a directive: "name arg1 arg2 arg3 ...;"
		if (*it == ";") {
			push_new_directive(temp, server_block._settings);
			continue;
		}
	// temp contains the start of a block: "name /path {..."
		else if (*it == "{") {
			++it;
			if (!push_new_block(temp, server_block._blocks, tokens, it, i))
				return false;
			continue;
		}
		else if (*it == "}") { // end of current global block
			++it;
			break;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------]
/** @brief Insert temp as a new setting
*
 * empty directives ("; ;") are ignored
*
 * if a directive appears multiple times, the first occurrence wins	---*/
static void	push_new_directive(v_str& temp, map_strstr& settings) {
// LOG_LOG("push_new_directive()")

	std::pair<std::string, std::string> rtrn;
	v_str::iterator itt = temp.begin();

	if (itt == temp.end())
		return;
// <directive name>
	rtrn.first = *itt;
	++itt;
// [optional arg]
	oss arg;
	for ( ; itt != temp.end(); ++itt)
		arg << *itt << " ";
	rtrn.second = trim_white(arg.str());
	settings.insert(rtrn);
}

//-----------------------------------------------------------------------------]
static bool	push_new_block(v_str& temp, std::vector<Settings::block>& settings, v_str& tokens, v_str::iterator& it_tokens, int i) {
// LOG_LOG("push_new_block()")

	Settings::block		b;
	if (!temp.size() || temp.size() > 2)
		return false;

	v_str::iterator it = temp.begin();
// set: name /path {...
	b.name = *it++;
	if (it != temp.end()) {
		b.path = *it++;
		b.hasPath = true;
	}

// loop over: { <directive name> [optional arg]
// 				<directive name> [arg1] [arg2] [arg3]
// 				<directive name> [empty] }
	v_str subblock_tokens;
	for ( ; it_tokens != tokens.end(); ++it_tokens) {
		if (*it_tokens == "{") { // parsing error
			LOG_ERROR("unexpected extra token '{' detected in block ending at line " << i)
			return false;
		}
		else if (*it_tokens == "}") {
			++it_tokens;
			break;
		}

		if (*it_tokens == ";") {
			push_new_directive(subblock_tokens, b.settings);
			subblock_tokens.clear();
			continue;
		}
		subblock_tokens.push_back(*it_tokens);
	}
	settings.push_back(b);
	return true;
}
