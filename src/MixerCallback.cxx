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
  if (sockfd < 0) {
    throw new system_error(errno, generic_category());
  }
}

void MixerCallback::close() {
  if (sockfd >= 0) {
    if (::close(sockfd)) {
      throw new system_error(errno, generic_category());
    }
    sockfd = -1;
  }
}

bool MixerCallback::opened() { return (!sockfd < 0); }

/**
 * onMixedAudioFrame 和 onMixedVideoFrame 回调不用加锁，是因为mixer
 * stop调用会join 混流线程的退出。mixer的stop 返回意味着mixer
 */
void MixerCallback::onMixedAudioFrame(TRTCAudioFrame *frame) {
  // cout << "onMixedAudioFrame ... " << endl;
  if (sockfd < 0) {
    return;
  }
  // cout << "onMixedAudioFrame ... send() ... addr=" << sendto_addr.sun_path
  //      << endl;
  ssize_t number_sent =
      sendto(sockfd, frame->data, frame->length, 0,
             (struct sockaddr *)&sendto_addr, sizeof(sendto_addr));
  // cout << "onMixedAudioFrame ... send() -> " << number_sent << endl;
  if (number_sent < 0) {
    switch (errno) {
    case ENOENT:
      break;
    case ECONNREFUSED:
      break;
    default: {
      throw new system_error(errno, generic_category());
    } break;
    }
  }
}

// /**
//  * \brief 1.2 回调混流后的视频帧，视频格式YUV420p。
//  * \param  frame 混后的视频帧。
//  * \return void.
//  */
// void MixerCallback::onMixedVideoFrame(TRTCVideoFrame *frame) {}

/**
 * \brief 1.3 内部错误回调。
 */
void MixerCallback::onError(int errcode, char *errmsg) {
  ostringstream oss;
  oss << "ITRTCMediaMixer error (" << errcode << ") " << errmsg;
  cerr << oss.str() << endl;
  throw new runtime_error(oss.str());
}
