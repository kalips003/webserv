#include "HttpObj.hpp"

#include "Tools1.hpp"
#include "Tools2.hpp"

#include <unistd.h>
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
temp_file& temp_file::operator=(const temp_file& other) {
	if (this != &other && other._fd >= 0) {
		_path = other._path;
		_fd = open(_path.c_str(), O_RDWR | O_CLOEXEC);
	}
	return *this;
}

#include <cerrno>
///////////////////////////////////////////////////////////////////////////////]
/** @brief Update temp_file with a unique temporary file.
 *
 * @param root_path           Pointer to a string specifying the directory where
 *  the temp file should be created. Checked for NULL, but should be valid
 *
 * @return true on success, false + ErrMsg on error.
 * @note - Created with flags: O_RDWR , O_CLOEXEC and permissions 0666
 * @note - Attempts up to MAX_TEMP_FILES_ALLOWED unique filenames before failing.	---*/
bool	temp_file::createTempFile(const std::string* root_path) {

	if (!root_path || _fd != -1)
		return false;

	if (access(root_path->c_str(), W_OK | X_OK) != 0) {
		oss msg; msg << "createTempFile(): Issue with temp directory (" << root_path << ")";
		printErr(msg.str().c_str());
		return false;
	}

	std::string file_begin = *root_path + "/webserv_tmp_";
	std::string temp_name;
	int fd;

	oss num;
	for (int i = 0; i < MAX_TEMP_FILES_ALLOWED; i++) {

		num.str(""); num.clear();
		num << i;
		temp_name = file_begin + num.str();

// FLAGS can be changed later with fcntl()
	// O_EXCL: if file already exist, fail
	// O_CLOEXEC → guarantees you don’t race another process/thread
		if ((fd = open(temp_name.c_str(), O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0666)) >= 0) {

			_path = temp_name;
			_fd = fd;
			return true;
		}
		else if (errno != EEXIST) {
			printErr("createTempFile(): open()");
			return false;
		}

	}
	printLog(WARNING, "Too many attempts at creating temp file failed", 1);
	return false;
}

/////////////////////////////////////////////////////////////////////////////]
// O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0666
bool	temp_file::openFile(const std::string& path, int flags, bool tmp) {

	if (path.empty() || _fd != -1)
		return false;

	int fd;
	if ((fd = open(path.c_str(), O_CLOEXEC | flags, 0666)) >= 0) {

		_path = path;
		_fd = fd;
		_delete = tmp;
		return true;
	}
	
	printErr("createTempFile(): open()");
	return false;
}


///////////////////////////////////////////////////////////////////////////////]
/**	Update the _info struct before a read from it */
bool temp_file::updateStat() {
	if (_path.empty()) {
		LOG_WARNING("updateStat(): trying to update with empty path");
		return true;
	}
	if(stat(_path.c_str(), &_info)) {
		printErr("updateStat(): stat()");
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////]
// Cannot update O_WRONLY O_RDONLY O_RDWR
bool temp_file::updateFlags(int flags_to_remove, int flags_to_add) {
	int flags = fcntl(_fd, F_GETFL);
	if (flags == -1) {
		printErr("updateFlags(): fcntl()");
		return false;
	}

	flags &= ~flags_to_remove;
	flags |= flags_to_add;

	if (fcntl(_fd, F_SETFL, flags) == -1) {
		printErr("updateFlags(): fcntl()");
		return false;
	}
	return true;
}


///////////////////////////////////////////////////////////////////////////////]
ssize_t	temp_file::getBodySize() {
	if (!updateStat()) return -1;
	return _info.st_size;
}

///////////////////////////////////////////////////////////////////////////////]
void	temp_file::closeTemp(bool del) {

	if (del)
		unlink(_path.c_str());
	_path.clear();

	if (_fd >= 0) {
		close (_fd);
		_fd = -1;	
	}
}

#include <fcntl.h>
///////////////////////////////////////////////////////////////////////////////]
/**	Reset the fd of the file to point at the start of it */
void	temp_file::resetFileFd() {
	lseek(_fd, 0, SEEK_SET);
}

///////////////////////////////////////////////////////////////////////////////]
///////////////////////////////////////////////////////////////////////////////]
std::ostream& operator<<(std::ostream& os, const temp_file& r) {

	os << C_542 "File (fd=" RESET << r._fd << C_542 ") _path: {\n" RESET << r._path << C_542 "}" RESET << std::endl;
	return os;
}
