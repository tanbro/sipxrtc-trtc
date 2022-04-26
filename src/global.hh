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
#include "UdsReader.hh"
#include "UdsWriter.hh"

extern std::mutex app_mtx;
extern ITRTCCloud *room;
extern ITRTCMediaMixer *mixer;

extern AudioRecvCallback audRecvCallback;
extern LogCallback logCallback;
extern MixerCallback mixerCallback;
extern RoomCallback roomCallback;

extern UdsReader* udsReader;
extern UdsWriter* udsWriter;

#endif
