#include "HttpObj.hpp"

#include <fcntl.h>

///////////////////////////////////////////////////////////////////////////////]
temp_file::~temp_file() {

	if (!_path.empty() && _delete) {
		unlink(_path.c_str());
		_path.clear();
	}
	if (_fd >= 0) {
		close(_fd);
		_fd = -1;
	}
}

///////////////////////////////////////////////////////////////////////////////]
/** copy the path, but open a new fd (O_RDWR) */
temp_file& temp_file::operator=(const temp_file& other) {
	if (this != &other && other._fd >= 0) {
		_path = other._path;
		_fd = open(_path.c_str(), O_RDWR | O_CLOEXEC);
		_delete = other._delete;
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////////]
temp_file& temp_file::operator<=(temp_file& other) {
	if (this != &other) {
		closeTemp(true);
		_fd = other._fd;
		_path = other._path;
		_delete = other._delete;
		
		other._fd = -1;
		other._path.clear();
		other._delete = false;
	}
	return *this;
}


///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
/** @return the size of the file if exist */
ssize_t	temp_file::getBodySize() {
	if (_fd < 0 || _path.empty()) return 0;
	if (!updateStat()) return -1;
	return _info.st_size;
}

///////////////////////////////////////////////////////////////////////////////]
/**	Update the _info struct before a read from it */
bool temp_file::updateStat() {
	if (_path.empty()) {
		LOG_WARNING("updateStat(): trying to update with empty path");
		return true;
	}
	if(stat(_path.c_str(), &_info)) {
		LOG_ERROR("updateStat(): stat()");
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////]
// Cannot update O_WRONLY O_RDONLY O_RDWR
bool temp_file::updateFlags(int flags_to_remove, int flags_to_add) {
	int flags = fcntl(_fd, F_GETFL);
	if (flags == -1) {
		LOG_ERROR("updateFlags(): fcntl()");
		return false;
	}

	flags &= ~flags_to_remove;
	flags |= flags_to_add;

	if (fcntl(_fd, F_SETFL, flags) == -1) {
		LOG_ERROR("updateFlags(): fcntl()");
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////]
/**	Reset the fd of the file to point at the start of it */
void	temp_file::resetFileFdBegining() {
	lseek(_fd, 0, SEEK_SET);
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, const temp_file& r) {

	os << C_542 "File (fd=" RESET << r._fd << C_542 ") _path: {\n" RESET << r._path << C_542 "}" RESET << std::endl;
	return os;
}
