#ifndef SERVERSETTINGS_HPP
#define SERVERSETTINGS_HPP

#include <map>
#include <vector>
#include <string>

#include "defines.hpp"
#include "_colors.h"

class ServerSettings;
extern ServerSettings g_settings;

class Server;
///////////////////////////////////////////////////////////////////////////////]
// name arg { set1 a; set2 b }
struct block {
    std::string                         name;
    std::string                         arg;
    std::map<std::string, std::string>  settings;
};
///////////////////////////////////////////////////////////////////////////////]


///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
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

public:
//-----------------------------------------------------------------------------]
	bool	parse_config_file(const char* confi_file);
	bool    check_settings( void );

//-----------------------------------------------------------------------------]
public:
///////////////////////////////////////////////////////////////////////////////]
/***  GETTERS  ***/
	int			getPortNum( void ) const { return _port_num; }
	std::string	getRoot( void ) const { return find_setting("root"); }
	std::string find_setting(const std::string& setting) const;
	std::string find_setting_inBlock(const std::string& set) const;
	const ServerSettings& getConstSettings() const { return *this; }

private:
	ServerSettings& getSettings() { return *this; }
/***  SETTERS  ***/
public:
	void	addSetting(std::string& name, std::string& value) { _global_settings[name] = value; }
	void	addBlock(block& b) { _block_settings.push_back(b); }

///////////////////////////////////////////////////////////////////////////////]

	friend class Server;
};

#endif
