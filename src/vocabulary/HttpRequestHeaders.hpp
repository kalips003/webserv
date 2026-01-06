#ifndef HTTPREQUESTHEADERS_HPP
#define HTTPREQUESTHEADERS_HPP

#include <vector>
#include <string>

///////////////////////////////////////////////////////////////////////////////]
/**
 * Initializer for the known request headers vector
 *
 * if first called, create the vector as a static
 * 
 * @return      a const ref to the vector
 */
const std::vector<std::string>& HTTP_Request_Headers_list();
///////////////////////////////////////////////////////////////////////////////]
/** Return the index in the headers vector coresponding to the input string
* 
* Return -1 if header isnt known */
int isHTTP_Request_HeadersValid(const std::string& s);

#endif