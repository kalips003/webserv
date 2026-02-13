#ifndef COOKIES_HPP
#define COOKIES_HPP

#include "Log.hpp"
#include "defines.hpp"

///////////////////////////////////////////////////////////////////////////////]
class Cookies {

public:
///////////////////////////////////////////////////////////////////////////////]
	struct data_cookies {
		// time_t	created_at;
		// time_t	last_seen;
		std::string		user;
		// bool	authenticated; //???
		v_str			options;
		map_strstr		_options;
		// json	
	};

private:
///////////////////////////////////////////////////////////////////////////////]
	std::string		_session_id;
	time_t			_created_at;
	time_t			_last_seen;
	data_cookies	_data;

public:
	Cookies();
	Cookies(const std::string& user = "");





static Cookies*	findLoggedUser(std::map<std::string, Cookies>& map, const std::string& to_find);
static std::string generateRandomID();
	std::string*	findInOptions();




};

#endif
