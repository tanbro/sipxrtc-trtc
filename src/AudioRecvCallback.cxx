#include "AudioRecvCallback.hxx"

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "global.hxx"

using namespace std;

void AudioRecvCallback::onRecvAudioFrame(const char *userId,
                                         TRTCAudioFrame *frame) {
  int trtc_errno;
  // assert(frame->sampleRate == 48000);
  // assert(frame->channel == 1);
  // assert(frame->length == 1920);

  if (mixer != nullptr) {
    lock_guard<std::mutex> lk(trtc_app_mutex);
    if ((trtc_errno = mixer->addAudioFrame(userId, frame))) {
      ostringstream oss;
      oss << "ITRTCMediaMixer::addAudioFrame() error " << trtc_errno;
      throw new runtime_error(oss.str());
    }
  }
}
