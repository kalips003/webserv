#ifndef TOOLS1_HPP
#define TOOLS1_HPP

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <unistd.h>
///////////////////////////////////////////////////////////////////////////////]
/**
 * Better Atoi
 *
 * @param input   String to convert into int
 * @param rtrn   ref to an int to fill with the result
 * @return      TRUE if the number is valid, FALSE otherwise (overflow, letters, ...)
 */
bool    atoi_v2(const std::string& input, int& rtrn);
bool	atoi_v2(const std::string& input, ssize_t& rtrn);

///////////////////////////////////////////////////////////////////////////////]
/**
 * Print perror and the given msg
 *
 * @param errmsg   String to print alongside the msg
 * @return      FALSE in all cases
 */
bool printErr(const char* errmsg);
///////////////////////////////////////////////////////////////////////////////]
/**
 * Trim white around the given string (" \t\n\r")
 *
 * @param s   Const String to trim
 * @return    trimmed string, empty "" if only whitespaces
 */
std::string trim_white(const std::string& s);
///////////////////////////////////////////////////////////////////////////////]
/**
 * Trim any characters present in the to_trim string, around the string s
 *
 * @param s   Const String to trim
 * @param to_trim   Set of characters to remove
 * @return    trimmed string, empty "" if only whitespaces
 */
std::string trim_any(const std::string& s, const char *to_trim);
///////////////////////////////////////////////////////////////////////////////]
/**
 * Split the given string on the delimit string arg
 *
 * @param s   Const String to split
 * @param delimit   String used as delimitor
 * @return    Vector of string, delimitor removed
 * @note	empty bits are not added to the vector		---*/
std::vector<std::string> splitOnDelimitor(const std::string& s, std::string delimit);
///////////////////////////////////////////////////////////////////////////////]
/**
 * Split the given string on whitespaces
 *
 * @param s   Const String to split
 * @return    Vector of string
 */
std::vector<std::string> splitOnWhite(const std::string& s);
///////////////////////////////////////////////////////////////////////////////]
/** Cpp version of itoa */
std::string itostr(int n);

///////////////////////////////////////////////////////////////////////////////]
/** Template Cpp version of itoa */
template <typename Num>
std::string itostr(Num n) {

	std::stringstream ss;
	ss << n;
	return ss.str();
}

///////////////////////////////////////////////////////////////////////////////]
/** Output function on std::cout
 *
 * @param lvl   INFO, ERROR, WARNING or DEBUG
 * @param s   string to output.
 * @param newline   true to output a newline after msg
 *
 * oss msg; msg << C_431 "" RESET; printLog(DEBUG, msg.str(), 1);
 */
void printLog(const std::string& lvl, const std::string& s, bool newline);

///////////////////////////////////////////////////////////////////////////////]
/** return a string ready to print the given fd in color  */
std::string printFd(int fd);

///////////////////////////////////////////////////////////////////////////////]
/** What Is It?
* @return index of c in dico, -1 if not found	*/
ssize_t wii(char c, const std::string& dico);

///////////////////////////////////////////////////////////////////////////////]
template <typename K, typename T>
std::ostream&	operator<<(std::ostream& os, const std::map<K, T>& map) {

	for (typename std::map<K, T>::const_iterator it = map.begin(); it != map.end(); ++it) {
		os << it->first << " = " << it->second << std::endl;
	}
	return os;
}

///////////////////////////////////////////////////////////////////////////////]
template <typename T>
std::ostream&	operator<<(std::ostream& os, const std::vector<T>& vector) {

	for (typename std::vector<T>::const_iterator it = vector.begin(); it != vector.end(); ++it) {
		os << *it << " ";
	}
	return os;
}


#endif