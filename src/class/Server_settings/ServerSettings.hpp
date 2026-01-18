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
	bool								hasPath;
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
class ServerSettings {

private:
///////////////////////////////////////////////////////////////////////////////]
	std::map<std::string, std::string>  _global_settings;
	std::vector<block>                  _block_settings;
	int                                 _port_num;
	std::string							_root;
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
	int					getPortNum( void ) const { return _port_num; }
	/**	@return /full/path/to/root (no trailing /) */
	std::string			getRoot( void ) const { return _root; }
//
	const ServerSettings& getConstSettings() const { return *this; }
private:
	ServerSettings& getSettings() { return *this; }
//	FIND IN THE BLOCKS
public:
	const std::string*				find_setting(const std::string& setting) const;
	const block*					find_global_block(const std::string& block_name) const;
	const std::vector<const block*>	find_arg_blocks(const std::string& block_name) const;
	const block*					find_arg_block_from_vector(const std::vector<const block*>& v, const std::string& arg_name) const;
	const std::string*				find_setting_in_block(const block* b, const std::string& setting) const;
	const std::string*				find_setting_in_blocks(const std::string& block_name, const std::string& arg, const std::string& setting) const;
/***  SETTERS  ***/
public:
	void	addSetting(std::string& name, std::string& value) { _global_settings[name] = value; }
	void	addBlock(block& b) { _block_settings.push_back(b); }
private:
	bool 	setRoot();
///////////////////////////////////////////////////////////////////////////////]

	friend class Server;
	friend std::ostream& operator<<(std::ostream& os, const ServerSettings& setting);

};

#endif
