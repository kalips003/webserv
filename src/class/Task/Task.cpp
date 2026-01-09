#include "Task.hpp"

#include "Connection.hpp"
#include "Server.hpp"

///////////////////////////////////////////////////////////////////////////////]
Task::Task(Connection& connec)
 : _request(connec.getRequest()), _answer(connec.getAnswer()), _status(0) {}

///////////////////////////////////////////////////////////////////////////////]
#include "Ft_Get.hpp"
Task* Task::createTask(const std::string& method, Connection& connec) {
    if (method == "GET")       return new Ft_Get(connec);
    // else if (method == "POST") return new Ft_post(req, ans);
    // else if (method == "PUT")  return new Ft_put(req, ans);
    // ... other methods
    else return NULL;  // unknown method → 405 or reject
}
