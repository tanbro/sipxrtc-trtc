#include "AudioRecvCallback.hxx"

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include "global.hxx"

using namespace std;

void AudioRecvCallback::onRecvAudioFrame(const char *userId,
                                         TRTCAudioFrame *frame) {
  // std::cout << "onRecvAudioFrame ... 0" << std::endl;
  lock_guard<std::mutex> lk(trtc_app_mutex);

  assert(frame->sampleRate == 48000);
  assert(frame->channel == 1);
  assert(frame->length == 1920);

  int errcode;
  if (mixer != nullptr) {
    // std::cout << ">>> mixer->addAudioFrame()" << std::endl;
    errcode = mixer->addAudioFrame(userId, frame);
    // std::cout << "<<< mixer->addAudioFrame()" << std::endl;
    if (errcode) {
      std::ostringstream oss;
      oss << "TXLiteAVSDK error on ITRTCMediaMixer::addAudioFrame. error_code="
          << errcode;
      cerr << oss.str() << endl;
      throw new runtime_error(oss.str());
    }
  }
}