#ifndef __TrtcCallback_h__
#define __TrtcCallback_h__

#include <iostream>

#include "TRTCCloud.h"

#include "Recorder.hxx"
#include "global.hxx"

class TrtcCallback : public ITRTCCloudCallback {

  void onError(TXLiteAVError errCode, const char *errMsg, void *arg) {
    std::cerr << "Error (" << errCode << ")" << errMsg << std::endl;
  };

  void onWarning(TXLiteAVWarning warningCode, const char *warningMsg,
                 void *arg) {
    std::cerr << "Warning (" << warningCode << ")" << warningMsg << std::endl;
  }

  void onEnterRoom(uint64_t elapsed) {
    std::cout << "EnterRoom: elapsed=" << elapsed << std::endl;
    users.clear();
  };

  void onExitRoom(int reason) {
    std::cout << "EnterRoom: reason=" << reason << std::endl;
    users.clear();
  };

  void onUserEnter(const char *userId) {
    std::cout << "UserEnter: userId=" << userId << std::endl;
    if (users.size() == 0) {
      trtc->setRemoteAudioRecvCallback(
          userId, TRTCAudioFrameFormat::TRTCAudioFrameFormat_PCM,
          new Recorder("/tmp/sipxrtp-trtc.sock"));
      users.insert(userId);
    }
  };

  void onUserExit(const char *userId, int reason) {
    std::cout << "UserExit: userId=" << userId << ", reason=" << reason
              << std::endl;
    auto it = users.find(userId);
    if (it != users.end()) {
      trtc->setRemoteAudioRecvCallback(
          userId, TRTCAudioFrameFormat::TRTCAudioFrameFormat_PCM, NULL);
      users.erase(it);
    }
  };
};

#endif
