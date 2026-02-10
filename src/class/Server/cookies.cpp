#include "Server.hpp"

///////////////////////////////////////////////////////////////////////////////]
struct Session {
    time_t      created_at;
    time_t      user;
    time_t      last_seen;
    bool        authenticated; //???
    v_str       other_settings;
    // json        
};

// GET /script/api42?login=loginname
/*
1) header no cookies
2) generate some id?


3) > send answer with Cookies-set: session_id=111; Path=/; HttpOnly; Max-Age=1800

4) client send: Cookies: session_id=111



*/

// void f() {
//     std::map<std::string, Session> cookies_user;
//     cookies_user.insert(newuser);


// }

#include <cstdlib>   // for rand(), srand()
#include <sstream>
std::string generateRandomID() {

    static bool seeded = false;
    if (!seeded) {
        srand(static_cast<unsigned int>(time(NULL) ^ getpid()));
        seeded = true;
    }

    oss os;
    os << std::hex
        << time(NULL)        // current time
        << rand()            // first random number
        << rand()            // second random number
        << reinterpret_cast<unsigned long>(&os); // pointer for extra entropy

    return os.str();
}