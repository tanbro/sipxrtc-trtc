#include "EventPub.hh"

#include <fcntl.h>

#include <climits>
#include <cstdio>
#include <cstring>

#include <glog/logging.h>

#include "utils.hh"

using namespace std;

EventPub::EventPub(const string &path) { _path = path; }

EventPub::~EventPub() {
  if (!(_fd < 0)) {
    close();
  }
}

int EventPub::open() {
  CHECK_GT(0, _fd);
  LOG(INFO) << "open(\"" << _path << "\") ... ";
  CHECK_ERR(_fd = ::open(_path.c_str(), O_WRONLY | O_NONBLOCK));
  LOG(INFO) << "fd: " << _fd;
  return _fd;
}

void EventPub::close() {
  CHECK_LE(0, _fd);
  CHECK_ERR(::close(_fd));
  _fd = -1;
}

int EventPub::getFd() { return _fd; }

void EventPub::pub(const string &msg) {
  CHECK_LE(0, _fd);
  ostringstream oss;
  oss << trimStr(msg) << endl;
  CHECK_GT(PIPE_BUF, oss.str().length());
  char *buf = (char *)calloc(oss.str().length() + 1, sizeof(char));
  strncpy(buf, oss.str().c_str(), oss.str().length());
  VLOG(1) << "pub: " << buf;
  CHECK_ERR(::write(_fd, buf, oss.str().length() + 1));
  free(buf);
}
