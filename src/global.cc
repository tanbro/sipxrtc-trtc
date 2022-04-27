#include "global.hh"

bool interrupted = false;

std::mutex app_mtx;

ITRTCCloud *room = nullptr;
ITRTCMediaMixer *mixer = nullptr;
ITRTCMediaRecorder *recorder = nullptr;

AudioRecvCallback audRecvCallback;
SdkLogger sdkLogger;
MixerCallback mixerCallback;
RoomCallback roomCallback;

UdsReader *udsReader = nullptr;
UdsWriter *udsWriter = nullptr;
