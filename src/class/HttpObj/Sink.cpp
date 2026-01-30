#include "HttpObj.hpp"

#include <unistd.h>

bool StringSink::write(const char* data, size_t len) {
	s.append(data, len);
	return true;
}
bool FileSink::write(const char* data, size_t len) {
	ssize_t r = ::write(f._fd, data, len);
	return r >= 0 && static_cast<size_t>(r) == len;
}
