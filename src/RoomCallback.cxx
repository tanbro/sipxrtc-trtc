
#include "RoomCallback.hxx"

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include "AudioRecvCallback.hxx"
#include "global.hxx"

using namespace std;

static Region dummy_region{0, 0, 0, 0, 0, REGION_OPTION_FILLMODE_FULL, 0};

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
};

void RoomCallback::onExitRoom(int reason) {
  /// TODO: 然后呢！
  cout << "[WARNING] onExitRoom: reason=" << reason << endl;
};

void RoomCallback::onUserEnter(const char *userId) {
  cerr << "[INFO   ] User " << userId << " entered the room" << endl;

  int err;
  {
    lock_guard<mutex> lk(trtc_app_mutex);
    /// IMPORTANT: 经测试，如果不设置该用户的视频混流 Region，则 MediaMixer
    /// 无法接受音频输入数据报
    if (mixer != nullptr) {
      mixer->setRegion(userId, &dummy_region);
      mixer->applyRegions();
    }
    err = room->setRemoteAudioRecvCallback(
        userId, TRTCAudioFrameFormat::TRTCAudioFrameFormat_PCM,
        &audRecvCallback);
  }
  if (err) {
    ostringstream oss;
    oss << "ITRTCCloud::setRemoteAudioRecvCallback() error " << err;
    cerr << oss.str() << endl;
    throw new runtime_error(oss.str());
  }
}

void RoomCallback::onUserExit(const char *userId, int reason) {
  cerr << "[INFO   ] User " << userId << " exited the room" << endl;
  int err;
  {
    lock_guard<mutex> lk(trtc_app_mutex);
    err = room->setRemoteAudioRecvCallback(
        userId, TRTCAudioFrameFormat::TRTCAudioFrameFormat_Unknown, NULL);
  }
  if (err) {
    ostringstream oss;
    oss << "ITRTCCloud::setRemoteAudioRecvCallback() error " << err;
    cerr << oss.str() << endl;
    throw new runtime_error(oss.str());
  }
}
