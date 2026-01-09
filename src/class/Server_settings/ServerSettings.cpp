#include "ServerSettings.hpp"

#include <iostream>

ServerSettings g_settings;

///////////////////////////////////////////////////////////////////////////////]
/** Getter for a setting
 *
 * @param set   Name of the setting to find
 * @return      String of the setting value, empty string "" otherwise		---*/
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
/** Getter for a block setting
 *
 * <TODO> empty for now			---*/
std::string ServerSettings::find_setting_inBlock(const std::string& set) const {
	(void)set;
	return "";
}