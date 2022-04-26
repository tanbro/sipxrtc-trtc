#include "UdsReader.hh"

#include <sys/socket.h>
#include <sys/stat.h>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <thread>

#include <glog/logging.h>

#include "global.hh"

using namespace std;

using TClock = chrono::high_resolution_clock;
using TDuration = chrono::duration<float, micro>;

UdsReader::UdsReader(const std::string &path) : path(path) {
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
}

int UdsReader::open() {
  UdsBase::open();
  struct stat statbuf;
  if (!stat(addr.sun_path, &statbuf)) {
    LOG(WARNING) << "unlink " << path;
    CHECK_ERR(unlink(addr.sun_path));
  }
  CHECK_ERR(bind(fd, (const sockaddr *)&addr, sizeof(addr)));
  LOG(INFO) << "bind " << fd << ":" << path;
  return fd;
}

void UdsReader::close() {
  UdsBase::close();
  struct stat statbuf;
  if (!stat(addr.sun_path, &statbuf)) {
    CHECK_ERR(unlink(addr.sun_path));
  }
}

ssize_t UdsReader::read() {
  CHECK_LT(0, fd);
  void *data = read_buffer;
  size_t length = sizeof(read_buffer);

  auto tsBegin = TClock::now();

  VLOG(6) << "[" << hex << this_thread::get_id() << "] "
          << ">>> recv("
          << "fd=" << hex << fd << dec << ", "
          << "data=" << data << ", "
          << "length=" << length << ")";
  ssize_t res = recv(fd, data, length, 0);
  if (res < 0) {
    if (errno != EWOULDBLOCK) {
      PCHECK(errno) << ": recv() failed: ";
    }
    return res;
  }
  VLOG(6) << "[" << hex << this_thread::get_id() << "] "
          << "<<< recv("
          << "fd=" << hex << fd << dec << ", "
          << "data=" << data << ", "
          << "length=" << length << ")"
          << " -> " << dec << res;
  ///
  resetTrtcAudFrm();
  trtcAudFrm.data = (uint8_t *)data;
  trtcAudFrm.length = length;
  CHECK_EQ(0, room->sendCustomAudioData(&trtcAudFrm));
  //
  TDuration elapsed = TClock::now() - tsBegin;
  VLOG_EVERY_N(3, 100) << "read()"
                       << " "
                       << "elapsed = " << elapsed.count() << " usec";
  return res;
}

void UdsReader::resetTrtcAudFrm() {
  trtcAudFrm.audioFormat = TRTCAudioFrameFormat::TRTCAudioFrameFormat_PCM;
  trtcAudFrm.data = NULL;
  trtcAudFrm.sampleRate = 48000;
  trtcAudFrm.channel = 1;
  trtcAudFrm.timestamp = 0;
}
