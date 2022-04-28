#include "EventPub.hh"

#include <fcntl.h>

#include <climits>
#include <cstdio>
#include <cstring>

#include <glog/logging.h>

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
  string s = msg;
  while (!s.empty() && isspace(s.back()))
    s.pop_back();
  s += "\n";
  CHECK_GT(PIPE_BUF, msg.length());
  char *buf = (char *)calloc(msg.length() + 1, sizeof(char));
  strncpy(buf, msg.c_str(), msg.length());
  CHECK_ERR(::write(_fd, buf, msg.length() + 1));
  free(buf);
}
