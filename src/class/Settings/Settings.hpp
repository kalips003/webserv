#ifndef SETTING_HPP
#define SETTING_HPP

#include "Log.hpp"
#include "defines.hpp"

#include <map>
#include <vector>
#include <string>
#include <unistd.h>

#include "_colors.h"

class Settings;
class Server;
extern Settings g_settings;

///////////////////////////////////////////////////////////////////////////////]


///////////////////////////////////////////////////////////////////////////////]

/*************************************]
# Global HTTP server
server {
    listen 80;
    server_name example.com;

    root /var/www/html;
    index index.html index.htm;

    location / {
        autoindex off;
    }

    location /images {
        autoindex on;
    }
}

# MIME types definition
mimetype {
    text/html        html htm;
    text/css         css;
    image/jpeg       jpeg jpg;
}

# Global settings
temp /tmp
**************************************/

///////////////////////////////////////////////////////////////////////////////]
class Settings {										  // SERVER SETTINGS //]

public:
	// DEFAULT DATA EACH PATHED BLOCK SHOULD HAVE
	struct location_data {
		std::string					root;
		std::vector<std::string>	index;
		bool						autoindex;
		std::string					post_policy;
		std::string					cgi_interpreter;
		std::vector<std::string>	allowed_methods;
		ssize_t						client_max_body_size;
		
		location_data() : autoindex(false), client_max_body_size(-1) {}
	};

	// name arg { set1 a; set2 b }
	struct block {
		location_data				data;
		std::string					name;
		std::string					path;
		map_strstr					settings;
		bool						hasPath;

		block() : hasPath(false)  {}
		bool operator==(const block& b) {
			return name == b.name && path == b.path;
		}
	};

	// Struct for one Server listen
	struct server_setting {
		std::string				_server_block_name; //	server {...
		std::string				_server_name; //		webcat.com
		std::string				_root; // 				www/webcat.com
		int						_port; // 				9999
		ssize_t					_client_max_body_size;
		location_data*			_root_location_data; // ptr to the location_data struct of root
	
		map_strstr				_settings;
		std::vector<block>		_blocks;

		server_setting() : _port(-1), _client_max_body_size(-1), _root_location_data(NULL) {}
	};


private:
///////////////////////////////////////////////////////////////////////////////]
	map_strstr											_global_settings;
	std::map<std::string, server_setting>		_global_blocks;

	std::string				_root; // path to the executable
	std::string				_temp_root; // path to the folder temp files
///////////////////////////////////////////////////////////////////////////////]
	// std::map<std::string, std::string>	_global_settings;
	// std::vector<block>					_block_settings;
	// int									_port_num;
	// ssize_t								_client_max_body_size;
	// location_data*						_root_location_data; // ptr to the location_data struct of root
	// std::string							_root; // root path of the server binary
	// std::string							_temp_root; // root path for temp files
///////////////////////////////////////////////////////////////////////////////]

public:
	Settings() {}
// static int			sanitizePath(std::string& path_to_fill, const std::string& given_path);
// static const block*	isLocationKnown(const std::string& given_path);
// static int 			getFullPath(std::string& path_to_fill, const std::string& sanitized);

//-----------------------------------------------------------------------------]

public:

	bool		parse_config_file(const char* confi_file);
private:
	bool 		setRoot();
	bool 		setTemp();
public:
	static bool	blockSetup(server_setting& a_global_block, std::string& root);


	static int					sanitizePath(std::string& path_to_fill, const std::string& given_path);
	static const block*			isLocationKnown(const std::string& given_path, const server_setting& a_global_block);
	static int 					getFullPath(std::string& path_to_fill, const std::string& sanitized, const Settings::server_setting& a_global_block);
	static const std::vector<const block*>
								rtrnMapOfMatches(Settings::block& for_this_block, Settings::server_setting& a_global_block);
// 	bool	check_settings( void );
// //
// 	void	default_settings_setup( void );
// 	void 	setAllBlockLocations();
// 	bool 	checkAnyRoot(std::string& some_root);
// //
// 	bool	setLocationData(block& b, const std::vector<const block*>& map_of_matches);

///////////////////////////////////////////////////////////////////////////////]
	/***  FIND IN THE BLOCKS  ***/

private:
	static std::string*						find_setting(const std::string& setting, server_setting& server_block);
	static std::vector<block*> 				find_arg_blocks(const std::string& block_name, server_setting& a_global_block);

public:
	static const std::vector<const block*>	find_arg_blocks(const std::string& block_name, const server_setting& a_global_block);
	const server_setting*					find_global_block(const std::string& block_name);
	// static const block*						find_global_block(const std::string& block_name);
	// static const std::vector<const block*>	find_arg_blocks(const std::string& block_name);
	// static const block*						find_arg_block_from_vector(const std::vector<const block*>& v, const std::string& arg_name);
	// static const std::string*				find_setting_in_block(const block* b, const std::string& setting);
	// static const std::string*				find_setting_in_blocks(const std::string& block_name, const std::string& arg, const std::string& setting);
private:
	// block* 							find_root_block();

///////////////////////////////////////////////////////////////////////////////]
	/***  GETTERS  ***/
public:
	const std::string*		isCGI(const std::string& path);
	// int						getPortNum( void ) const { return _port_num; }
	const std::string&		getRoot( void ) const { return _root; }
	const std::string&		getTempRoot() const { return _temp_root; }
	// const location_data*	getRootLocationBlock() const { return _root_location_data; }
	// const Settings&	getConstSettings() const { return *this; }
	// ssize_t					getMaxBodySize() const { return _client_max_body_size; }
private:
	// Settings& 		getSettings() { return *this; }

///////////////////////////////////////////////////////////////////////////////]
	/***  SETTERS  ***/
public:
	// void	addSetting(std::string& name, std::string& value) { _global_settings[name] = value; }
	// void	addBlock(block& b) { _block_settings.push_back(b); }
private:

///////////////////////////////////////////////////////////////////////////////]

	/***  FRIENDS  ***/
	friend class Server;
	friend std::ostream&	operator<<(std::ostream& os, const Settings& setting);
	friend  bool			default_settings_setup(Settings::server_setting& global_block);

};

// typedef std::map<std::string, Settings::server_setting> map_global_b;

std::ostream& operator<<(std::ostream& os, const Settings& s);
std::ostream& operator<<(std::ostream& os, const Settings::server_setting& b);
std::ostream& operator<<(std::ostream& os, const Settings::location_data& b);
std::ostream& operator<<(std::ostream& os, const Settings::block& b);

#endif
