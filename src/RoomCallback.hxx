#ifndef __sipxtrtc_RoomCallback_h__
#define __sipxtrtc_RoomCallback_h__ 1

#include <iostream>

#include "ITRTCMediaMixer.h"
#include "TRTCCloud.h"

class RoomCallback : public ITRTCCloudCallback {
public:
  void onError(TXLiteAVError errCode, const char *errMsg, void *arg);

  void onWarning(TXLiteAVWarning warningCode, const char *warningMsg,
                 void *arg);

  void onEnterRoom(uint64_t elapsed);
  void onExitRoom(int reason);

  void onUserEnter(const char *userId);
  void onUserExit(const char *userId, int reason);

};

#endif
