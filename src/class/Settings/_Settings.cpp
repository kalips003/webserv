#include "Settings.hpp"

#include "Tools1.hpp"

#include <stdlib.h> // strtol()

///////////////////////////////////////////////////////////////////////////////]
/** Decode and validate URL path.
 *  Expands %XX sequences, rejects malformed encodings and control characters.
 *  Example: "/img%2Ftest.png" → "/img/test.png"
 *  @return 0 on success, 400 on invalid path		---*/
int Settings::sanitizePath(std::string& path_to_fill, const std::string& given_path) {

	for (size_t i = 0; i < given_path.size(); ++i) {

		if (given_path[i] == '%') {
			if (i + 2 >= given_path.size()) {
				LOG_ERROR("Truncated %XX sequence: ..." << &given_path[i]);
				return 400;
			}

			char hex[3];
			hex[0] = given_path[i + 1];
			hex[1] = given_path[i + 2];
			hex[2] = '\0';
			char* end = NULL;

			long val = strtol(hex, &end, 16);
			if (*end != '\0') { // invalid hex digits
				LOG_ERROR("Invalid %XX sequence: %" << hex);
				return 400;
			}
			if (val <= 31 || val > 127) { // control chars
				LOG_ERROR("Control / Invalid character in path: %" << hex);
				return 400;
			}
			path_to_fill += static_cast<char>(val);
			i += 2;
		} else {
			char c = given_path[i];
			if ((c >= 0 && c <= 31) || c == 127) { // control chars
				LOG_ERROR("Control character in path: (" << c << ")");
				return 400;
			}
			path_to_fill += c;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////]
/** Find best matching location block for a request path.
 *  Uses longest-prefix match with boundary checks.
 *  Example: "/folder/sub/file" → matches "location /folder/sub"
 *  @return pointer to matched location_data, or NULL if none		
//  @note Default root block is expected to exist ---*/
const Settings::block* Settings::isLocationKnown(const std::string& given_path, const Settings::server_setting& a_global_block) {

	const std::vector<const block *> locations = Settings::find_arg_blocks("location", a_global_block);

	const block*	best_match = NULL;
	size_t			best_len   = 0;
	for (size_t i = 0; i < locations.size(); ++i) {
		const block* b = locations[i];
		const std::string& loc = b->path; // e.g. "/images"

		// must be a prefix
		if (given_path.compare(0, loc.size(), loc) != 0) // compare: given_path[0 .. loc.size()-1]  vs  loc
			continue;

		// boundary check: "/img" must not match "/images"
		if (loc != "/" && given_path.size() > loc.size() && given_path[loc.size()] != '/')
			continue;

		// longest match wins
		if (loc.size() > best_len) {
			best_match = b;
			best_len = loc.size();
		}
	}
	return best_match;
}

///////////////////////////////////////////////////////////////////////////////]
/** @brief Build the full filesystem path from sanitize path.
 * 
 * Remove the block path and replace with root
 * Ex:
 *   Request path: "/folder/..///./file%201.txt" > "<location_root>/file 1.txt"
 *
 * @param path_to_fill  Reference to a string where the full path will be stored.
 * @return int          0 on success, or 403 if the path attempts to escape the root.	---*/
int Settings::getFullPath(std::string& path_to_fill, const std::string& sanitized, const Settings::server_setting& a_global_block) {

	const block* location_block = isLocationKnown(sanitized, a_global_block);
	if (!location_block) {
		LOG_ERROR("CAN'T FIND LOCATION BLOCK: " << sanitized)
		return 500; // could also be 404;
	}

	std::vector<std::string> stack;
	std::vector<std::string> v = splitOnDelimitor(sanitized, "/");
// == ["images", "..", ".", "icons", "file.png"]

// remove /location/path from sanitized
	std::vector<std::string> v_location = splitOnDelimitor(location_block->path, "/");
	v.erase(v.begin(), v.begin() + v_location.size());
// == ["..", ".", "icons", "file.png"]

// check for root escaping
	for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); ++it) {
		if (*it == ".") continue;
		if (*it == "..") {
			if (stack.empty()) {
				LOG_ERROR("Escape attempt from root: " C_431 << sanitized << RESET "]");
				return 403;
			}
			stack.pop_back();
		}
		else
			stack.push_back(*it);
	}

// concatenate back = "/location_folder/file"
	std::string ressource;
	for (std::vector<std::string>::iterator it = stack.begin(); it != stack.end(); ++it) {
		ressource += *it;
		if (it + 1 != stack.end()) ressource += "/";
	}

// add the root of block location_folder = "/given_root/file"
	ressource.insert(0, location_block->data.root + "/");

	path_to_fill = ressource;
	return 0;
}


///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, const Settings& setting) {

	os << RED << "\t\t### WEBSERV CONFIG FILE ###\n\n";
	os << C_214 "_root to webserv: \"" RESET << setting._root << C_214 "\"" RESET << std::endl;
	os << C_214 "_temp_root: \"" RESET << setting._temp_root << C_214 "\"" RESET << std::endl;

	os << RED << "\t\t### GLOBAL SETTINGS ###\n";
	for (map_strstr::const_iterator it = setting._global_settings.begin(); it != setting._global_settings.end(); ++it) {
		os << C_140 << it->first << RESET " => \"" C_140 << it->second << RESET "\"" << std::endl;
	}
	os << RED << "\t\t### GLOBAL BLOCKS ###\n";
	for (std::map<std::string, Settings::server_setting>::const_iterator it = setting._global_blocks.begin(); it != setting._global_blocks.end(); ++it) {
		os << RED << "# " << it->first << " #: {\n";
		os << C_404 << it->second;
		os << C_305 << "\t_settings: {\n" RESET;
		for (map_strstr::const_iterator it2 = it->second._settings.begin(); it2 != it->second._settings.end(); ++it2) {
			os << "\t\t" C_140 << it2->first << RESET " => \"" C_140 << it2->second << RESET "\"" << std::endl;
		}
		os << C_305 << "\t}\n" RESET;

		os << C_305 << "\t_blocks: {\n" RESET;
		for (std::vector<Settings::block>::const_iterator it2 = it->second._blocks.begin(); it2 != it->second._blocks.end(); ++it2) {
			os << "\t\t" << *it2;
		}
		os << C_305 << "\t}\n" RED "  }" << std::endl;
	}

	return os;
}

//-----------------------------------------------------------------------------]
	// LOCATION_DATA
std::ostream& operator<<(std::ostream& os, const Settings::location_data& b) {
	os << C_045 "location_data {" RESET << std::endl;

	os << "\troot: " << b.root << std::endl;
	os << "\tindex: [ " << b.index << "]\n";

	os << "\tautoindex: " << (b.autoindex ? "true" : "false") << std::endl;
	os << "\tpost_policy: " << b.post_policy << std::endl;
	os << "\tcgi_interpreter: " << b.cgi_interpreter << std::endl;

	os << "\tallowed_methods: [ " << b.allowed_methods << "]\n";

	os << "\tclient_max_body_size: " << b.client_max_body_size << std::endl;

	os << C_045 "}" RESET << std::endl;
	return os;
}
//-----------------------------------------------------------------------------]
	// BLOCK
std::ostream& operator<<(std::ostream& os, const Settings::block& b) {

	os <<  "\"" C_431 << b.name << RESET "\" \"" C_431 << b.path << RESET "\"{" << std::endl;
	os << C_544 << b.settings << RESET;
	os << C_431 "}" RESET << std::endl;

	os << "DATA: \n" << b.data << std::endl;

	return os;

}
//-----------------------------------------------------------------------------]
	// SERVER SETTINGS
std::ostream& operator<<(std::ostream& os, const Settings::server_setting& b) {

	os << "_server_block_name: " << b._server_block_name << std::endl;
	os << "_server_name: " << b._server_name << std::endl;
	os << "_root: " << b._root << std::endl;
	os << "_port: " << b._port << std::endl;
	os << "_client_max_body_size: " << b._client_max_body_size << std::endl;
	// os << "_root_location_data: " << *b._root_location_data << std::endl;
	os << "\t_settings: " << b._settings << std::endl;
	// os << "_blocks: " << b._blocks << std::endl;

	return os;
}
