#include "MixerCallback.hh"

#include <sys/socket.h>

#include <sstream>
#include <thread>

#include <glog/logging.h>

#include "global.hh"

using namespace std;

void MixerCallback::onMixedAudioFrame(TRTCAudioFrame *frame) {
  udsWriter->write(frame->data, frame->length);
}

/**
 * \brief 1.3 内部错误回调。
 */
void MixerCallback::onError(int errcode, char *errmsg) {
  ostringstream oss;
  oss << "ITRTCMediaMixer internal error " << errcode << ": " << errmsg;
  LOG(FATAL) << oss.str();
}
