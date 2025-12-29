#include "ServerSettings.hpp"

#include <iostream>

///////////////////////////////////////////////////////////////////////////////]
std::string ServerSettings::find_setting(const std::string& setting) const {

	map_strstr::const_iterator it = _global_settings.begin();
	it = _global_settings.find(setting);
	if (it == _global_settings.end()) {
		std::cerr << RED "setting not found: " RESET << setting << std::endl;
		return "";
	}
	else 
		return it->second;
}

///////////////////////////////////////////////////////////////////////////////]
std::string ServerSettings::find_setting_inBlock(const std::string& set) const {

	return "";
}