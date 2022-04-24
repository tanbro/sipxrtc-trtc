#include "AudioRecvCallback.hh"

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <glog/logging.h>

#include "global.hh"

using namespace std;

void AudioRecvCallback::onRecvAudioFrame(const char *userId,
                                         TRTCAudioFrame *frame) {
  int trtc_errno;
  // assert(frame->sampleRate == 48000);
  // assert(frame->channel == 1);
  // assert(frame->length == 1920);

  if (mixer != nullptr) {
    lock_guard<std::mutex> lk(trtc_app_mutex);
    trtc_errno = mixer->addAudioFrame(userId, frame);
  }
  CHECK_EQ(0, trtc_errno) << ": ITRTCMediaMixer::addAudioFrame(userId=\""
                          << userId << "\") failed.";
}
