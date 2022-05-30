#include "RoomCallback.hh"

#include <vector>

#include <glog/logging.h>

#include "AudioRecvCallback.hh"
#include "global.hh"

using namespace std;

static Region dummy_region{
    0, 0, 0, 0, 0, RegionOption::REGION_OPTION_FILLMODE_FULL, 0};

/// SDK不可恢复的错误!
void RoomCallback::onError(TXLiteAVError errCode, const char *errMsg,
                           void *arg) {
  switch (errCode) {
  case ERR_KICKOUT_BY_SERVER:
    LOG(ERROR) << errCode << ": " << errMsg;
    /// ATTENTION: 被踢，就直接退出程序！
    interrupted = true;
    break;

  default:
    LOG(FATAL) << errCode << ": " << errMsg;
    break;
  }
};

void RoomCallback::onWarning(TXLiteAVWarning warningCode,
                             const char *warningMsg, void *arg) {
  LOG(WARNING) << warningCode << ": " << warningMsg;
}

void RoomCallback::onEnterRoom(uint64_t elapsed) {
  /// TODO: 进去了房间才能继续后面的操作！
  lock_guard<mutex> lk(app_mtx);
  LOG(WARNING) << "onEnterRoom: elapsed=" << elapsed;
  _entered = true;
  if (eventPub) {
    ostringstream oss;
    oss << "onEnterRoom: " << elapsed;
    eventPub->pub(oss.str());
  }
}

void RoomCallback::onExitRoom(int reason) {
  lock_guard<mutex> lk(app_mtx);
  LOG(WARNING) << "onExitRoom: reason=" << reason;
  _entered = false;
  /// ATTENTION: 如果不在房间里，就直接退出程序！
  interrupted = true;
  if (eventPub) {
    ostringstream oss;
    oss << "onExitRoom: " << reason;
    eventPub->pub(oss.str());
  }
};

void RoomCallback::onUserEnter(const char *userId) {
  lock_guard<mutex> lk(app_mtx);
  LOG(INFO) << "onUserEnter: " << userId;
  userIdSet.insert(userId);
  if (_subFlag) {
    subone(string(userId));
  }
  if (eventPub) {
    ostringstream oss;
    oss << "onUserEnter: " << userId;
    eventPub->pub(oss.str());
  }
}

void RoomCallback::onUserExit(const char *userId, int reason) {
  lock_guard<mutex> lk(app_mtx);
  LOG(INFO) << "onUserExit: " << userId;
  userIdSet.erase(userId);
  unsubone(string(userId));

  if (eventPub) {
    ostringstream oss;
    oss << "onUserExit: " << userId;
    eventPub->pub(oss.str());
  }
}

void RoomCallback::subone(const std::string &userId) {
  CHECK_NOTNULL(room);
  CHECK_NOTNULL(mixer);
  LOG(INFO) << "setRemoteAudioRecvCallback on user " << userId;
  /// IMPORTANT: 经测试，如果不设置该用户的视频混流 Region，则 MediaMixer
  /// 无法接受音频输入数据报
  CHECK_EQ(0, mixer->setRegion(userId.c_str(), &dummy_region));
  CHECK_EQ(0, mixer->applyRegions());
  CHECK_EQ(0,
           room->setRemoteAudioRecvCallback(
               userId.c_str(), TRTCAudioFrameFormat::TRTCAudioFrameFormat_PCM,
               &audRecvCallback));
}
void RoomCallback::unsubone(const std::string &userId) {
  CHECK_NOTNULL(room);
  CHECK_NOTNULL(mixer);
  LOG(INFO) << "unsetRemoteAudioRecvCallback on user " << userId;
  CHECK_EQ(0, mixer->setRegion(userId.c_str(), nullptr));
  CHECK_EQ(0, mixer->applyRegions());
  CHECK_EQ(0, room->setRemoteAudioRecvCallback(
                  userId.c_str(),
                  TRTCAudioFrameFormat::TRTCAudioFrameFormat_Unknown, nullptr));
}

void RoomCallback::suball() {
  lock_guard<mutex> lk(app_mtx);
  if (!_entered) {
    LOG(ERROR) << "not entered room yet.";
    return;
  }
  _subFlag = true;
  for (auto it = userIdSet.begin(); it != userIdSet.end(); ++it) {
    subone(*it);
  }
}

void RoomCallback::unsuball() {
  lock_guard<mutex> lk(app_mtx);
  if (!_entered) {
    LOG(ERROR) << "not entered room yet.";
    return;
  }
  _subFlag = false;
  for (auto it = userIdSet.begin(); it != userIdSet.end(); ++it) {
    unsubone(*it);
  }
}
