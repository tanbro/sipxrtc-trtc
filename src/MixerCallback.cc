#include "MixerCallback.hh"

#include <sys/socket.h>

#include <sstream>
#include <thread>

#include <glog/logging.h>

using namespace std;

void MixerCallback::open(const string &path) {
  CHECK_GT(0, sockfd);
  CHECK_NOTNULL(memset(&sendto_addr, 0, sizeof(sendto_addr)));
  sendto_addr.sun_family = AF_UNIX;
  strncpy(sendto_addr.sun_path, path.c_str(), sizeof(sendto_addr.sun_path) - 1);
  CHECK_ERR(sockfd = socket(AF_UNIX, SOCK_DGRAM, 0));
  LOG(INFO) << ": bind " << sockfd << ":" << path;
}

void MixerCallback::close() {
  if (sockfd >= 0) {
    return;
  }
  if (sockfd >= 0) {
    DVLOG(1) << "close " << sockfd;
    PCHECK(0 == ::close(sockfd));
    sockfd = -1;
  }
}

bool MixerCallback::opened() { return (sockfd >= 0); }

void MixerCallback::onMixedAudioFrame(TRTCAudioFrame *frame) {
  // VLOG_EVERY_N(6, 100) << "onMixedAudioFrame";
  VLOG(6) << "[" << this_thread::get_id() << "]"
          << " "
          << ">>> onMixedAudioFrame ...";
  if (sockfd < 0) {
    VLOG(6) << "[" << this_thread::get_id() << "]"
            << " "
            << "<<< onMixedAudioFrame";
    return;
  }
  VLOG(6) << "[" << this_thread::get_id() << "]"
          << " "
          << " ... onMixedAudioFrame ... sendto ...";
  ssize_t n_bytes =
      sendto(sockfd, frame->data, frame->length, 0,
             (struct sockaddr *)&sendto_addr, sizeof(sendto_addr));
  VLOG(6) << "[" << this_thread::get_id() << "]"
          << " "
          << " ... onMixedAudioFrame ... sendto -> " << n_bytes;
  if (n_bytes < 0) {
    switch (errno) {
    case ENOENT:
      break;
    case ECONNREFUSED:
      break;
    default: {
      PCHECK(errno) << ": sendto() failed: ";
    } break;
    }
  }
  VLOG_IF_EVERY_N(3, n_bytes > 0, 100) << "send() -> " << n_bytes << " bytes";
  VLOG(6) << "[" << this_thread::get_id() << "]"
          << " "
          << "<<< onMixedAudioFrame " << n_bytes << " bytes was sent";
}

/**
 * \brief 1.3 内部错误回调。
 */
void MixerCallback::onError(int errcode, char *errmsg) {
  ostringstream oss;
  oss << "ITRTCMediaMixer internal error " << errcode << ": " << errmsg;
  LOG(FATAL) << oss.str();
}
