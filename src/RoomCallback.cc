
#include "RoomCallback.hh"

#include <thread>

#include <glog/logging.h>

#include "AudioRecvCallback.hh"
#include "global.hh"

using namespace std;

static Region dummy_region{
    0, 0, 0, 0, 0, RegionOption::REGION_OPTION_FILLMODE_FULL, 0};

/// SDK不可恢复的错误!
void RoomCallback::onError(TXLiteAVError errCode, const char *errMsg,
                           void *arg) {
  LOG(FATAL) << errCode << ": " << errMsg;
};

void RoomCallback::onWarning(TXLiteAVWarning warningCode,
                             const char *warningMsg, void *arg) {
  LOG(WARNING) << warningCode << ": " << warningMsg;
}

void RoomCallback::onEnterRoom(uint64_t elapsed) {
  /// TODO: 进去了房间才能继续后面的操作！
  lock_guard<mutex> lk(app_mtx);
  LOG(WARNING) << "[" << this_thread::get_id() << "]"
               << " "
               << "EnterRoom elapsed=" << elapsed;
  _entered = true;
}

void RoomCallback::onExitRoom(int reason) {
  lock_guard<mutex> lk(app_mtx);
  LOG(WARNING) << "[" << this_thread::get_id() << "]"
               << " "
               << "ExitRoom reason=" << reason;
  _entered = false;
  /// TODO: 然后呢！
  interrupted = true;
};

void RoomCallback::onUserEnter(const char *userId) {
  lock_guard<mutex> lk(app_mtx);
  LOG(INFO) << "[" << this_thread::get_id() << "]"
            << " "
            << "UserEnter: " << userId;
  CHECK_NOTNULL(mixer);
  /// IMPORTANT: 经测试，如果不设置该用户的视频混流 Region，则 MediaMixer
  /// 无法接受音频输入数据报
  if (mixer != nullptr) {
    CHECK_EQ(0, mixer->setRegion(userId, &dummy_region));
    CHECK_EQ(0, mixer->applyRegions());
  }
  CHECK_EQ(0, room->setRemoteAudioRecvCallback(
                  userId, TRTCAudioFrameFormat::TRTCAudioFrameFormat_PCM,
                  &audRecvCallback));
}

void RoomCallback::onUserExit(const char *userId, int reason) {
  lock_guard<mutex> lk(app_mtx);
  LOG(INFO) << "[" << this_thread::get_id() << "]"
            << " "
            << "UserExit: " << userId;
  CHECK_EQ(0, room->setRemoteAudioRecvCallback(
                  userId, TRTCAudioFrameFormat::TRTCAudioFrameFormat_Unknown,
                  nullptr));
  CHECK_NOTNULL(mixer);
  if (mixer != nullptr) {
    CHECK_EQ(0, mixer->setRegion(userId, nullptr));
    CHECK_EQ(0, mixer->applyRegions());
  }
}
