#ifndef HTTPANSWERHEADERS_HPP
#define HTTPANSWERHEADERS_HPP


#include <vector>
#include <string>

///////////////////////////////////////////////////////////////////////////////]
/**
 * Initializer for the known answer headers vector
 *
 * if first called, create the vector as a static
 * 
 * @return      a const ref to the vector
 */
const std::vector<std::string>& HTTP_Answer_Headers_list();



#endif