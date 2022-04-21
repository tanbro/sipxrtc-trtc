#ifndef __sipxtrtc_RecorderCallback_h
#define __sipxtrtc_RecorderCallback_h 1

#include "ITRTCMediaRecorder.h"

class RecorderCallback : public ITRTCMediaRecorderCallback {
public:
  virtual ~RecorderCallback() {}

  /**
   * \brief 1.1 录制完成回调
   *
   * \param userId        用户ID/流id
   *
   * \param output        录制文件类型，纯音频文件 output = OUTFILE_TYPE_AUDIO
   *
   * \param filepath 录制文件路径，相对于dir参数所指路径，dir
   * 参数通过ITRTCMediaRecorder 的 setParam的dir参数进行设置的。
   *
   * \return void
   */
  virtual void onFinished(const char *userId, int output,
                          const char *filepath){};
};

#endif