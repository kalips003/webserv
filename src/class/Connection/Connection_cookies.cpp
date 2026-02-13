#include "Connection.hpp"

#include "Tools1.hpp"
///////////////////////////////////////////////////////////////////////////////]
int	Connection::handle_cookies() {

	std::string* cookie = _request.find_in_headers("Cookie");
	if (cookie) { // cookie is found
		_this_user = Cookies::findLoggedUser(_cookies, *cookie);
		_data._this_user = _this_user;
		
	}
	else { // cookie not found
	
	
	
	}

	// std::string path = _request.getPath();
	// std::string path_path = path.substr(0, path.find_first_of("?"));
	// std::string path_query = path.substr(path.find_first_of("?") + 1);
	// if (path_path == known_cookie) {
	// 	// handle
	// 	return Connection::SENDING;
	// }
	// else
		return 0;

}

// void createNewUserCookie() {

// 	map_strstr headers;
// /**
// | Attribute           | Meaning                        |
// | ------------------- | ------------------------------ |
// | `session_id=abc123` | key=value                      |
// | `Path=/`            | cookie valid for entire site   |
// | `HttpOnly`          | JS cannot access it (security) |
// | `Secure`            | only via HTTPS                 |
// | `Max-Age=3600`      | expires in 1 hour              |
// | `Expires=DATE`      | absolute expiration            |
// | `SameSite=Lax`      | CSRF protection                |
// **/
// 	std::string s = "session_id=abc123xyz; Path=/; HttpOnly; SameSite=Lax";
// 	std::string s_minimum = "session_id=RANDOM_ID; Path=/; HttpOnly";
// 	headers.addtoHeader("Set-Cookie", s);

// 	std::string receive_from_client = "Cookie: session_id=abc123xyz; theme=dark; lang=en";
// 	map_strstr	options;

// 	v_str v = splitOnDelimitor(receive_from_client, ";");
// 	for (v_str::iterator it  = v.begin(); it != v.end(); ++it) {
// 		*it = trim_white(*it);
// 		size_t pos = (*it).find_first_of("=");
// 		std::string arg = (*it).substr(0, pos);
// 		std::string value;
// 		if (pos == std::string::npos)
// 			continue;
// 		value = trim_any((*it).substr(pos + 1), "\"\'");
// 		options[arg] = value;
// 	}

// }