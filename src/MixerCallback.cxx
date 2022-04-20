#include "MixerCallback.hxx"

#include <unistd.h>

#include <sys/socket.h>

#include <cerrno>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <system_error>

using namespace std;

void MixerCallback::open(const string &path) {
  if (!sockfd < 0) {
    cerr << "already opened" << endl;
    return;
  }
  memset(&sendto_addr, 0, sizeof(sendto_addr));
  sendto_addr.sun_family = AF_UNIX;
  strncpy(sendto_addr.sun_path, path.c_str(), sizeof(sendto_addr.sun_path) - 1);
  sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    throw new system_error(errno, generic_category());
  }
}

void MixerCallback::close() {
  if (sockfd != -1) {
    if (::close(sockfd)) {
      throw new system_error(errno, generic_category());
    }
    sockfd = -1;
  }
}

bool MixerCallback::opened() { return (sockfd != -1); }

void MixerCallback::onMixedAudioFrame(TRTCAudioFrame *frame) {
  if (sockfd == -1) {
    return;
  }
  ssize_t n_bytes =
      sendto(sockfd, frame->data, frame->length, 0,
             (struct sockaddr *)&sendto_addr, sizeof(sendto_addr));
  if (n_bytes == -1) {
    switch (errno) {
    case ENOENT:
      break;
    case ECONNREFUSED:
      break;
    default:
      throw new system_error(errno, generic_category());
      break;
    }
  }
}

/**
 * \brief 1.3 内部错误回调。
 */
void MixerCallback::onError(int errcode, char *errmsg) {
  ostringstream oss;
  oss << "ITRTCMediaMixer internal error " << errcode << ": " << errmsg;
  throw new runtime_error(oss.str());
}
