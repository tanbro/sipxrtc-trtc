#ifndef __global_hxx__h__
#define __global_hxx__h__ 1

#include <mutex>
#include <string>

#include <ITRTCMediaMixer.h>
#include <ITRTCMediaRecorder.h>
#include <TRTCCloud.h>

#include "AudioRecvCallback.hh"
#include "EventPub.hh"
#include "MixerCallback.hh"
#include "RoomCallback.hh"
#include "SdkLogger.hh"
#include "UdsReader.hh"
#include "UdsWriter.hh"

extern bool interrupted;

extern std::mutex app_mtx;
extern ITRTCCloud *room;
extern ITRTCMediaMixer *mixer;

extern AudioRecvCallback audRecvCallback;
extern SdkLogger sdkLogger;
extern MixerCallback mixerCallback;
extern RoomCallback roomCallback;


extern UdsReader *udsReader;
extern UdsWriter *udsWriter;

extern EventPub *eventPub;

#endif
