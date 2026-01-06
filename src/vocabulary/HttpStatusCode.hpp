#ifndef HTTPSTATUSCODE_HPP
#define HTTPSTATUSCODE_HPP

#include <string>
#include <map>

///////////////////////////////////////////////////////////////////////////////]
/**
 * Initializer for the HTTP status code map
 *
 * if first called, create the map as a static
 * 
 * @return      a const ref to the map
 */
const std::map<int, std::string>&   http_status_map();
///////////////////////////////////////////////////////////////////////////////]
/** Return the string msg associated with the input code
* 
* Return empty string "" if code invalid */
std::string   return_http_from_code(int code);

#endif