#include "Task.hpp"

#include "Connection.hpp"
#include "Server.hpp"

///////////////////////////////////////////////////////////////////////////////]
Task::Task(Connection& connec, Server& s) : _request(connec.getRequest()), _answer(connec.getAnswer()), _settings(s.getSettings()), _status(0) {}


///////////////////////////////////////////////////////////////////////////////]
#include "Ft_Get.hpp"
Task* createTask(const std::string& method, Connection& connec, Server& s) {
    if (method == "GET")       return new Ft_Get(connec, s);
    // else if (method == "POST") return new Ft_post(req, ans);
    // else if (method == "PUT")  return new Ft_put(req, ans);
    // ... other methods
    else return NULL;  // unknown method → 405 or reject
}
