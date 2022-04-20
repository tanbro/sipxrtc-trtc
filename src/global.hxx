#ifndef __global_hxx__h__
#define __global_hxx__h__ 1

#include <mutex>
#include <string>

#include "ITRTCMediaMixer.h"
#include "ITRTCMediaRecorder.h"
#include "TRTCCloud.h"

#include "AudioRecvCallback.hxx"
#include "LogCallback.hxx"
#include "MixerCallback.hxx"
#include "RecorderCallback.hxx"
#include "RoomCallback.hxx"

extern std::mutex trtc_app_mutex;
extern ITRTCCloud *room;
extern ITRTCMediaMixer *mixer;
extern ITRTCMediaRecorder *recorder;

extern AudioRecvCallback audRecvCallback;
extern LogCallback logCallback;
extern MixerCallback mixerCallback;
extern RoomCallback roomCallback;
extern RecorderCallback recorderCallback;

#endif
