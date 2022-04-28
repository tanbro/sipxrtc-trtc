#ifndef __sipxtrtc_RoomCallback_h__
#define __sipxtrtc_RoomCallback_h__ 1

#include <iostream>
#include <set>

#include <ITRTCMediaMixer.h>
#include <TRTCCloud.h>

class RoomCallback : public ITRTCCloudCallback {
private:
  bool _entered = false;
  bool _subFlag = false;
  std::set<std::string> userIdSet;

  // 注意这两个不是线程安全的
  void subone(const std::string &userId);
  void unsubone(const std::string &userId);

public:
  void onError(TXLiteAVError errCode, const char *errMsg, void *arg);

  void onWarning(TXLiteAVWarning warningCode, const char *warningMsg,
                 void *arg);

  void onEnterRoom(uint64_t elapsed);
  void onExitRoom(int reason);

  void onUserEnter(const char *userId);
  void onUserExit(const char *userId, int reason);

  bool getEntered() { return _entered; };

  /**
   * @brief 订阅所有远端用户的声音输入
   *
   */
  void suball();

  void unsuball();
};

#endif
