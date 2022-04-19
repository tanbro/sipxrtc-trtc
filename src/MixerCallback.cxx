#include "MixerCallback.hxx"

#include <errno.h>

MixerCallback::MixerCallbace() {
  sendto_addr = (sockaddr_un *)calloc(1, sizeof(sockaddr_un));
  sendto_addr->sun_family = AF_UNIX;
  strncpy(sendto_addr->sun_path, path.c_str(),
          sizeof(sendto_addr->sun_path) - 1);

  sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    perror(strerror(errno));
  }
}

MixerCallback::~MixerCallback() {}

/**
 * \brief 1.1 回调混流后的音频帧，音频格式PCM。单声道 48kHz采样率
 * 16bit深度。20ms帧长（960个采样点，字节长度1920） \param  frame
 * 混后的音频帧。 \return void.
 */
void MixerCallback::onMixedAudioFrame(TRTCAudioFrame *frame) {}

/**
 * \brief 1.2 回调混流后的视频帧，视频格式YUV420p。
 * \param  frame 混后的视频帧。
 * \return void.
 */
void MixerCallback::onMixedVideoFrame(TRTCVideoFrame *frame) {}

/**
 * \brief 1.3 内部错误回调。
 */
void MixerCallback::onError(int errcode, char *errmsg) {}