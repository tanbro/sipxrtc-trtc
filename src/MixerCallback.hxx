#ifndef sipxtrtc_MixerCallback_h__
#define sipxtrtc_MixerCallback_h__ 1

#include <sys/socket.h>
#include <sys/un.h>

#include "ITRTCMediaMixer.h"
#include "TRTCCloud.h"

class MixerCallback : public ITRTCMediaMixerCallback {
private:
  sockaddr_un *sendto_addr;
  int sockfd = -1;

public:
  MixerCallback();
  virtual ~MixerCallback();

  /**
   * \brief 1.1 回调混流后的音频帧，音频格式PCM。单声道 48kHz采样率
   * 16bit深度。20ms帧长（960个采样点，字节长度1920） \param  frame
   * 混后的音频帧。 \return void.
   */
  virtual void onMixedAudioFrame(TRTCAudioFrame *frame);

  /**
   * \brief 1.2 回调混流后的视频帧，视频格式YUV420p。
   * \param  frame 混后的视频帧。
   * \return void.
   */
  virtual void onMixedVideoFrame(TRTCVideoFrame *frame);

  /**
   * \brief 1.3 内部错误回调。
   */
  virtual void onError(int errcode, char *errmsg);
};

#endif