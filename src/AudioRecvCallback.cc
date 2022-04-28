#include "AudioRecvCallback.hh"

#include <thread>

#include <glog/logging.h>

#include "global.hh"

using namespace std;

void AudioRecvCallback::onRecvAudioFrame(const char *userId,
                                         TRTCAudioFrame *frame) {
  CHECK_EQ(frame->sampleRate, 48000);
  CHECK_EQ(frame->channel, 1);
  CHECK_EQ(frame->length, 1920);
  VLOG(6) << "[" << this_thread::get_id() << "]"
          << "onRecvAudioFrame... ";
  CHECK_NOTNULL(mixer);
  if (mixer != nullptr) {
    lock_guard<std::mutex> lk(app_mtx);
    VLOG(6) << "[" << this_thread::get_id() << "]"
            << "onRecvAudioFrame... >>> mixer->addAudioFrame()";
    int status = mixer->addAudioFrame(userId, frame);
    LOG_IF(ERROR, status);
    VLOG(6) << "[" << this_thread::get_id() << "]"
            << "onRecvAudioFrame... <<< mixer->addAudioFrame()";
  }
}
