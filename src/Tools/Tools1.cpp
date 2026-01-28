#include "Tools1.hpp"

#include "_colors.h"

#include <iostream>
#include <climits>
#include <cstdlib>

#include <stdio.h> // perror
#include <cerrno> // perror

#include <sstream>

#include <defines.hpp>
///////////////////////////////////////////////////////////////////////////////]
bool	atoi_v2(const std::string& input, int& rtrn) {

	char* end = NULL;
	long num = strtol(input.c_str(), &end, 10);
	if (*end != '\0' || num > INT_MAX || num < INT_MIN) {
		std::cerr << RED "Input number invalid: " RESET << input << std::endl;
		return false;
	}
	rtrn = static_cast<int>(num);
	return true;
}

///////////////////////////////////////////////////////////////////////////////]
bool	atoi_v2(const std::string& input, ssize_t& rtrn) {

	char* end = NULL;
	long num = strtol(input.c_str(), &end, 10);
	if (*end != '\0' || errno == ERANGE || num < -1 || num > SSIZE_MAX) {
		std::cerr << RED "Input number invalid: " RESET << input << std::endl;
		return false;
	}
	rtrn = static_cast<ssize_t>(num);
	return true;
}

///////////////////////////////////////////////////////////////////////////////]
bool printErr(const char* errmsg) {

	std::cerr << ERROR << RED "System error - " RESET;
	perror(errmsg);
	// std::cerr << C_412 "; [errno]: " RESET << errno << std::endl;
	return false;
}


void printLog(const std::string& lvl, const std::string& s, bool newline) {

	if (lvl == DEBUG && !DEBUG_MODE)
		return;
	if (lvl == ERROR)
		std::cerr << lvl << s;
	else
		std::cout << lvl << s;
	if (newline)
		std::cout << std::endl;
}

///////////////////////////////////////////////////////////////////////////////]
std::string trim_white(const std::string& s) {

    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) 
        return ""; // all whitespace
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

///////////////////////////////////////////////////////////////////////////////]
std::string trim_any(const std::string& s, const char *to_trim) {

    size_t start = s.find_first_not_of(to_trim);
    if (start == std::string::npos) 
        return "";
    size_t end = s.find_last_not_of(to_trim);
    return s.substr(start, end - start + 1);
}

///////////////////////////////////////////////////////////////////////////////]
std::vector<std::string> splitOnDelimitor(const std::string& s, std::string delimit) {

	std::vector<std::string> rtrn;
	std::string line;
	size_t pos1 = 0;
	size_t pos2;

	while ((pos2 = s.find(delimit, pos1)) != std::string::npos) {
		line = s.substr(pos1, pos2 - pos1);
		if (!line.empty())
			rtrn.push_back(line);
		pos1 = pos2 + delimit.size();
	}
	line = s.substr(pos1);
	if (!line.empty())
		rtrn.push_back(line); 

	return rtrn;
}

///////////////////////////////////////////////////////////////////////////////]
std::vector<std::string> splitOnWhite(const std::string& s) {

    std::vector<std::string> rtrn;
    std::stringstream       ss(s);
    std::string line;
    while (ss >> line)
        rtrn.push_back(line);

    return rtrn;
}

///////////////////////////////////////////////////////////////////////////////]
std::string itostr(int n) {

    std::stringstream ss;
    ss << n;
    return ss.str();
}

///////////////////////////////////////////////////////////////////////////////]
std::string printFd(int fd) {
	oss colored_fd("\033[38;5;");
	int color = 16 + ((fd + 32) % 240);
	colored_fd << color << "m" << fd << RESET;
	return colored_fd.str();
}