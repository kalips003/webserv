#ifndef HTTPMETHODS_HPP
#define HTTPMETHODS_HPP

#include <vector>
#include <string>
#include "defines.hpp"

///////////////////////////////////////////////////////////////////////////////]
/**
 * Initializer for the HTTP methods vector
 *
 * if first called, create the vector as a static
 * 
 * @return      a const ref to the vector
 */
const std::vector<std::string>& http_method_list();
///////////////////////////////////////////////////////////////////////////////]
/** Return the enum HttpMethod coresponding to the input string
* 
* Return num HttpMethod: INVALID (-1) if method isnt known */
HttpMethod isMethodValid(const std::string& s);


#endif