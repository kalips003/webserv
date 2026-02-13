#include "Cookies.hpp"

#include <ctime>

///////////////////////////////////////////////////////////////////////////////]
/**
1) > rewrite path:
GET /index.html > /logged/index.html
simple, handled by connection

2) > method update the html
index.html: Hello {{USERNAME}} > updated. if placeholder exist

3) > cgi handleing
GET /index.cgi > cgi handle everything depending on knownuser / default

4)
if (_this_user)
    allow access to /dashboard
else
    return 403 or redirect to /login


5) 
What Real Servers Do

Modern servers:

Static files → unchanged

Dynamic content → handled by backend (CGI, PHP, Node, etc.)

Server itself does NOT rewrite HTML


**/
///////////////////////////////////////////////////////////////////////////////]
Cookies::Cookies() {

	_created_at = std::time(NULL);
	_last_seen = _created_at;
}

Cookies::Cookies(const std::string& user) : _session_id(user) {

	_created_at = std::time(NULL);
	_last_seen = _created_at;
}

///////////////////////////////////////////////////////////////////////////////]












#include <unistd.h>	// getpid()
#include <cstdlib>   // for rand(), srand()
#include <sstream>
///////////////////////////////////////////////////////////////////////////////]
std::string Cookies::generateRandomID() {

	static bool seeded = false;
	if (!seeded) {
		srand(static_cast<unsigned int>(time(NULL) ^ getpid()));
		seeded = true;
	}

	oss os;
	os << std::hex
		<< time(NULL)		// current time
		<< rand()			// first random number
		<< rand()			// second random number
		<< reinterpret_cast<unsigned long>(&os); // pointer for extra entropy

	return os.str();
}

///////////////////////////////////////////////////////////////////////////////]
Cookies*	Cookies::findLoggedUser(std::map<std::string, Cookies>& map, const std::string& to_find) {

	std::map<std::string, Cookies>::iterator it = map.find(to_find);
	if (it == map.end()) {
		LOG_DEBUG(C_511 "Cookie not known: " RESET << to_find);
		return NULL;
	}
	else 
		return &(it->second);
	// if time out, pop, and create new?
}
