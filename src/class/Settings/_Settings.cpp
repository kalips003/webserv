#include "Settings.hpp"

#include "Tools1.hpp"
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
		os << it->name << ": \"" << it->path << "\" {" << std::endl;
		for (map_strstr::const_iterator it2 = it->settings.begin(); it2 != it->settings.end(); ++it2) {
			os << it2->first << ": " << it2->second << std::endl;
		}
		os << "}\n" << std::endl;
	}
	os << C_214 "_global_settings: " RESET << setting._global_settings << std::endl;
	os << C_214 "_global_blocks: " RESET << setting._global_blocks << std::endl;

	return os;
}

//-----------------------------------------------------------------------------]
	// LOCATION_DATA
std::ostream& operator<<(std::ostream& os, const Settings::location_data& b) {
	os << C_045 "location_data {" RESET << std::endl;

	os << "\troot: " << b.root << std::endl;
	os << "\tindex: [ " << b.index << "]";

	os << "\tautoindex: " << (b.autoindex ? "true" : "false") << std::endl;
	os << "\tpost_policy: " << b.post_policy << std::endl;
	os << "\tcgi_interpreter: " << b.cgi_interpreter << std::endl;

	os << "\tallowed_methods: [ " << b.allowed_methods << "]";

	os << "\tclient_max_body_size: " << b.client_max_body_size << std::endl;

	os << C_045 "}" RESET << std::endl;
	return os;
}
//-----------------------------------------------------------------------------]
	// BLOCK
std::ostream& operator<<(std::ostream& os, const Settings::block& b) {

	os << C_431 << b.name << C_410 " \"" << b.path << "\" RESET {" << std::endl;
	os << b.settings;
	os << "}" << std::endl;

	os << "DATA: \n" << b.data << std::endl; // requires operator<< for location_data

	return os;

}
//-----------------------------------------------------------------------------]
	// SERVER SETTINGS
std::ostream& operator<<(std::ostream& os, const Settings::server_setting& b) {

	os << "_server_name: " << b._server_name << std::endl;
	os << "_root: " << b._root << std::endl;
	os << "_port: " << b._port << std::endl;
	os << "_client_max_body_size: " << b._client_max_body_size << std::endl;
	// os << "_root_location_data: " << *b._root_location_data << std::endl;
	os << "_settings: " << b._settings << std::endl;
	os << "_blocks: " << b._blocks << std::endl;

	return os;
}
