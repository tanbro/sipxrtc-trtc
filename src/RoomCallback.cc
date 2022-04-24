
#include "RoomCallback.hh"

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include "AudioRecvCallback.hh"
#include "global.hh"

using namespace std;

static Region dummy_region{
    0, 0, 0, 0, 0, RegionOption::REGION_OPTION_FILLMODE_FULL, 0};

void RoomCallback::onError(TXLiteAVError errCode, const char *errMsg,
                           void *arg) {
  ostringstream oss;
  oss << "[  ERROR] (" << errCode << ")" << errMsg;
  cerr << oss.str() << endl;
  throw new runtime_error(oss.str());
};

void RoomCallback::onWarning(TXLiteAVWarning warningCode,
                             const char *warningMsg, void *arg) {
  cerr << "[WARNING] (" << warningCode << ")" << warningMsg << endl;
}

void RoomCallback::onEnterRoom(uint64_t elapsed) {
  /// TODO: 进去了房间才能继续后面的操作！
  cout << "[INFO   ] onEnterRoom: elapsed=" << elapsed << endl;
  _entered = true;
};

void RoomCallback::onExitRoom(int reason) {
  _entered = false;
  /// TODO: 然后呢！
  cout << "[WARNING] onExitRoom: reason=" << reason << endl;
};

void RoomCallback::onUserEnter(const char *userId) {
  int trtc_errno;
  {
    lock_guard<mutex> lk(trtc_app_mutex);
    /// IMPORTANT: 经测试，如果不设置该用户的视频混流 Region，则 MediaMixer
    /// 无法接受音频输入数据报
    if (mixer != nullptr) {
      if ((trtc_errno = mixer->setRegion(userId, &dummy_region))) {
        ostringstream oss;
        oss << "ITRTCMediaMixer::setRegion(\"" << userId << "\", "
            << &dummy_region << ") error " << trtc_errno;
        throw new runtime_error(oss.str());
      }
      if ((trtc_errno = mixer->applyRegions())) {
        ostringstream oss;
        oss << "ITRTCMediaMixer::applyRegions() error " << trtc_errno;
        throw new runtime_error(oss.str());
      }
    }
    if ((trtc_errno = room->setRemoteAudioRecvCallback(
             userId, TRTCAudioFrameFormat::TRTCAudioFrameFormat_PCM,
             &audRecvCallback))) {
      ostringstream oss;
      oss << "ITRTCCloud::setRemoteAudioRecvCallback(\"" << userId << "\", "
          << &audRecvCallback << ") error " << trtc_errno;
      throw new runtime_error(oss.str());
    }
  }
}

void RoomCallback::onUserExit(const char *userId, int reason) {
  int trtc_errno;
  {
    lock_guard<mutex> lk(trtc_app_mutex);
    if ((trtc_errno = room->setRemoteAudioRecvCallback(
             userId, TRTCAudioFrameFormat::TRTCAudioFrameFormat_Unknown,
             nullptr))) {
      ostringstream oss;
      oss << "ITRTCCloud::setRemoteAudioRecvCallback(\"" << userId
          << "\", nullptr) error " << trtc_errno;
      throw new runtime_error(oss.str());
    }
    if (mixer != nullptr) {
      if ((trtc_errno = mixer->setRegion(userId, nullptr))) {
        ostringstream oss;
        oss << "ITRTCMediaMixer::setRegion(\"" << userId
            << "\", nullptr) error " << trtc_errno;
        throw new runtime_error(oss.str());
      }
      if ((trtc_errno = mixer->applyRegions())) {
        ostringstream oss;
        oss << "ITRTCMediaMixer::applyRegions() error " << trtc_errno;
        throw new runtime_error(oss.str());
      }
    }
  }
}
