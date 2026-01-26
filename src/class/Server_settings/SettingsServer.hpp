#ifndef SETTINGSSERVER_HPP
#define SETTINGSSERVER_HPP

#include <map>
#include <vector>
#include <string>
#include <unistd.h>

#include "defines.hpp"
#include "_colors.h"

class SettingsServer;
extern SettingsServer g_settings;

class Server;
///////////////////////////////////////////////////////////////////////////////]
// DEFAULT DATA EACH PATHED BLOCK SHOULD HAVE
struct location_data {
	std::string					root;
	std::vector<std::string>	index;
	bool						autoindex;
	std::string					post_policy;
	std::string					cgi_interpreter;
	std::vector<std::string>	allowed_methods;
	ssize_t						client_max_body_size;
};

// name arg { set1 a; set2 b }
struct block {
	location_data						data;
	std::string							name;
	std::string							path;
	std::map<std::string, std::string>  settings;
	bool								hasPath;

	bool operator==(const block& b) {
		return name == b.name && path == b.path;
	}
};

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
class SettingsServer {

private:
///////////////////////////////////////////////////////////////////////////////]
	std::map<std::string, std::string>	_global_settings;
	std::vector<block>					_block_settings;
	int									_port_num;
	ssize_t								_client_max_body_size;
	location_data*						_root_location_data; // ptr to the location_data struct of root
	std::string							_root; // root path of the server binary
	std::string							_temp_root; // root path for temp files
///////////////////////////////////////////////////////////////////////////////]

public:
	SettingsServer() : _port_num(-1), _client_max_body_size(-1), _root_location_data(NULL) {}


//-----------------------------------------------------------------------------]

private:
	bool	parse_config_file(const char* confi_file);
	bool	check_settings( void );
//
	void	default_settings_setup( void );
	bool 	setRoot();
	bool 	setTemp();
	void 	setAllBlockLocations();
	bool 	checkAnyRoot(std::string& some_root);
//
	bool	setLocationData(block& b, const std::vector<const block*>& map_of_matches);

///////////////////////////////////////////////////////////////////////////////]
	/***  FIND IN THE BLOCKS  ***/
public:
	const std::string*				find_setting(const std::string& setting) const;
	const block*					find_global_block(const std::string& block_name) const;
	const std::vector<const block*>	find_arg_blocks(const std::string& block_name) const;
	const block*					find_arg_block_from_vector(const std::vector<const block*>& v, const std::string& arg_name) const;
	const std::string*				find_setting_in_block(const block* b, const std::string& setting) const;
	const std::string*				find_setting_in_blocks(const std::string& block_name, const std::string& arg, const std::string& setting) const;
private:
	block* 							find_root_block();

///////////////////////////////////////////////////////////////////////////////]
	/***  GETTERS  ***/
public:
	int						getPortNum( void ) const { return _port_num; }
	std::string				getRoot( void ) const { return _root; }
	const SettingsServer&	getConstSettings() const { return *this; }
	const location_data*	getRootLocation() const { return _root_location_data; }
	std::string				getTempLocation() const { return _temp_root; }
private:
	SettingsServer& 		getSettings() { return *this; }

///////////////////////////////////////////////////////////////////////////////]
	/***  SETTERS  ***/
public:
	void	addSetting(std::string& name, std::string& value) { _global_settings[name] = value; }
	void	addBlock(block& b) { _block_settings.push_back(b); }
private:

///////////////////////////////////////////////////////////////////////////////]

	/***  FRIENDS  ***/
	friend class Server;
	friend std::ostream& operator<<(std::ostream& os, const SettingsServer& setting);

};

std::ostream& operator<<(std::ostream& os, const location_data& b);
std::ostream& operator<<(std::ostream& os, const block& b);

#endif
