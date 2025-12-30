#ifndef SERVERSETTINGS_HPP
#define SERVERSETTINGS_HPP

#include <map>
#include <vector>
#include <string>

#include "defines.hpp"
#include "_colors.h"

/**
 * Settings of the Server
 *
 * _global_settings: map of 
 "name value"
 *
 * _block_settings: map of 
 "name path { set1 a; set2 b }"
 */
struct ServerSettings {

private:
///////////////////////////////////////////////////////////////////////////////]
	std::map<std::string, std::string>  _global_settings;
	std::vector<block>                  _block_settings;
	int                                 _port_num;
///////////////////////////////////////////////////////////////////////////////]

public:
	ServerSettings() : _port_num(-1) {}

///////////////////////////////////////////////////////////////////////////////]
// SETTERS / GETTERS
	int		getPortNum( void ) { return _port_num; }
	void	addSetting(std::string& name, std::string& value) { _global_settings[name] = value; }
	void	addBlock(block& b) { _block_settings.push_back(b); }
///////////////////////////////////////////////////////////////////////////////]
/**
 * Getter for a setting
 *
 * @param set   Name of the setting to find
 * @return      String of the setting value, empty string "" otherwise
 */
	std::string find_setting(const std::string& setting) const;
///////////////////////////////////////////////////////////////////////////////]
/**
 * Getter for a block setting
 *
 * <TODO> empty for now 
 */
	std::string find_setting_inBlock(const std::string& set) const;
///////////////////////////////////////////////////////////////////////////////]
/**
 * MAIN function parsing the infile.conf
 *
 * DOES NOT check for the validity of the settings 
 *
 * @param confi_file   Valid char* of the path to the config file
 *
 * @return      FALSE on any parsing error, TRUE otherwise
 *
 */
	bool	parse_config_file(const char* confi_file);
///////////////////////////////////////////////////////////////////////////////]
/**
 * Check that the config file has the minimum settings
 , set them to default if missing
 *
 * Also set the _port_num from which the server will listen (default 8080)
 *
 * DOES NOT YET check for the validity of the others settings 
 *
 * <TODO> check if the settings directory can be opened
 * @return      FALSE on any error, TRUE otherwise
 *
 */
	bool    check_settings( void );
};

#endif