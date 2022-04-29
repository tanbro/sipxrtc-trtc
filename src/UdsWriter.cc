#include "UdsWriter.hh"

#include <sys/socket.h>

#include <chrono>
#include <cstring>

#include <glog/logging.h>

using namespace std;

using TClock = chrono::high_resolution_clock;
using TDuration = chrono::duration<float, micro>;

UdsWriter::UdsWriter(const std::string &path) : path(path) {
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
}

ssize_t UdsWriter::write(void *data, size_t length) {
  CHECK_LT(0, fd);
  auto tsBegin = TClock::now();
  VLOG(6) << ">>> sendto("
          << "fd=" << hex << fd << dec << ", "
          << "data=" << data << ", "
          << "length=" << length << ", "
          << "addr=\"" << path << "\""
          << ")";
  ssize_t res =
      sendto(fd, data, length, 0, (struct sockaddr *)&addr, sizeof(addr));
  if (res < 0) {
    switch (errno) {
    case ENOENT:
      break;
    case ECONNREFUSED:
      break;
    default:
      PCHECK(errno) << ": sendto() error: ";
      break;
    }
  }
  VLOG(6) << "<<< sendto("
          << "fd=" << hex << fd << dec << ", "
          << "data=" << data << ", "
          << "length=" << length << ", "
          << "addr=\"" << path << "\""
          << ")"
          << " -> " << res;
  TDuration elapsed = TClock::now() - tsBegin;
  VLOG_EVERY_N(3, 100) << "write()"
                       << " " << length << " bytes"
                       << " "
                       << "elapsed = " << elapsed.count() << " usec";

  return res;
}
