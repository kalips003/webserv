#include "webserv.hpp"

///////////////////////////////////////////////////////////////////////////////]

///////////////////////////////////////////////////////////////////////////////]
Task* createTask(const std::string& method, http_request& req, http_answer& ans) {
    if (method == "GET")       return new Ft_get(req, ans);
    // else if (method == "POST") return new Ft_post(req, ans);
    // else if (method == "PUT")  return new Ft_put(req, ans);
    // ... other methods
    else return NULL;  // unknown method → 405 or reject
}


std::ostream& operator<<(std::ostream& os, const connection& c) {

    os << C_152 "Client n." RESET << c._client_fd;
    // os << C_152 "Client n." RESET << c._client_fd << std::endl;
    // os << C_434 "addr: " RESET << c._client_addr.sin_addr.s_addr << std::endl;
    return os;
}



