#ifndef __TrtcCallback_h__
#define __TrtcCallback_h__ 1

#include <iostream>

#include "TRTCCloud.h"

#include <cassert>
#include <map>
#include <mutex>
#include <string>

#include "Receiver.hxx"
#include "global.hxx"

class TrtcCallback : public ITRTCCloudCallback {

  std::map<std::string, Receiver *> usersReceivers;
  std::mutex usersMtx;

  void onError(TXLiteAVError errCode, const char *errMsg, void *arg) {
    std::cerr << "Error (" << errCode << ")" << errMsg << std::endl;
  };

  void onWarning(TXLiteAVWarning warningCode, const char *warningMsg,
                 void *arg) {
    std::cerr << "Warning (" << warningCode << ")" << warningMsg << std::endl;
  }

  void onEnterRoom(uint64_t elapsed) {
    std::cout << "EnterRoom: elapsed=" << elapsed << std::endl;
  };

  void onExitRoom(int reason) {
    std::cout << "EnterRoom: reason=" << reason << std::endl;
    {
      std::lock_guard<std::mutex> lk(usersMtx);
      for (auto iter = usersReceivers.begin(); iter != usersReceivers.end();
           ++iter) {
        trtc->setRemoteAudioRecvCallback(
            iter->first.c_str(), TRTCAudioFrameFormat::TRTCAudioFrameFormat_PCM,
            NULL);
        delete iter->second;
      }
      usersReceivers.clear();
    }
  };

  void onUserEnter(const char *userId) {
    std::cout << "UserEnter: userId=" << userId << std::endl;
    {
      std::lock_guard<std::mutex> lk(usersMtx);
      assert(usersReceivers.find(userId) == usersReceivers.end());
      Receiver *receiver = new Receiver();
      trtc->setRemoteAudioRecvCallback(
          userId, TRTCAudioFrameFormat::TRTCAudioFrameFormat_PCM, receiver);
      usersReceivers[userId] = receiver;
    }
  };

  void onUserExit(const char *userId, int reason) {
    std::cout << "UserExit: userId=" << userId << ", reason=" << reason
              << std::endl;
    {
      std::lock_guard<std::mutex> lk(usersMtx);
      auto iter = usersReceivers.find(userId);
      assert(iter != usersReceivers.end());
      trtc->setRemoteAudioRecvCallback(
          userId, TRTCAudioFrameFormat::TRTCAudioFrameFormat_PCM, NULL);
      usersReceivers.erase(iter);
      delete iter->second;
    }
  };
};

#endif
