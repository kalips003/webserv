#include "HttpObj.hpp"

#include "Tools1.hpp"

///////////////////////////////////////////////////////////////////////////////]
httpObj::~httpObj() {
	if (!_tmp_body_path.empty()) unlink(_tmp_body_path.c_str());
	if (_fd_body >= 0) close(_fd_body);
}