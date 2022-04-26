#include "global.hh"

std::mutex app_mtx;

ITRTCCloud *room = nullptr;
ITRTCMediaMixer *mixer = nullptr;
ITRTCMediaRecorder *recorder = nullptr;

AudioRecvCallback audRecvCallback;
LogCallback logCallback;
MixerCallback mixerCallback;
RoomCallback roomCallback;

UdsReader *udsReader = nullptr;
UdsWriter *udsWriter = nullptr;
