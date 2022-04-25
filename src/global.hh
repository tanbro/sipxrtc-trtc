#ifndef __global_hxx__h__
#define __global_hxx__h__ 1

#include <mutex>
#include <string>

#include <ITRTCMediaMixer.h>
#include <ITRTCMediaRecorder.h>
#include <TRTCCloud.h>

#include "AudioRecvCallback.hh"
#include "LogCallback.hh"
#include "MixerCallback.hh"
#include "RoomCallback.hh"

extern std::mutex trtc_app_mutex;
extern ITRTCCloud *room;
extern ITRTCMediaMixer *mixer;
extern ITRTCMediaRecorder *recorder;

extern AudioRecvCallback audRecvCallback;
extern LogCallback logCallback;
extern MixerCallback mixerCallback;
extern RoomCallback roomCallback;

#endif
