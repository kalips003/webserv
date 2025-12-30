#ifndef TOOLS1_HPP
#define TOOLS1_HPP

#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////]
/**
 * Better Atoi
 *
 * @param input   String to convert into int
 * @param rtrn   ref to an int to fill with the result
 * @return      TRUE if the number is valid, FALSE otherwise (overflow, letters, ...)
 */
bool    atoi_v2(const std::string& input, int& rtrn);
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
 */
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

    std::ostringstream oss;
    oss << n;
    return oss.str();
}

#endif