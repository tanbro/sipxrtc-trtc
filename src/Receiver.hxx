#ifndef __sipxtrtc_Recorder_h__
#define __sipxtrtc_Recorder_h__ 1

#include <sys/un.h>

#include "TRTCCloud.h"

class Receiver : public ITRTCAudioRecvCallback {
private:


public:
  Receiver();
  ~Receiver();

  /// @name 自定义音频帧接收回调
  /// @{
  /**
   * \brief 6.2 通过 setRemoteAudioRecvCallback 接口设置音频数据回调
   *
   * \param userId     用户标识
   * \param frame      音频帧数据
   */
  void onRecvAudioFrame(const char *userId, TRTCAudioFrame *frame);

  static Receiver *getInstance();
};

#endif
