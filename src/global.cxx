#include "global.hxx"

std::mutex trtc_app_mutex;

ITRTCCloud *room = nullptr;
ITRTCMediaMixer *mixer = nullptr;
ITRTCMediaRecorder *recorder = nullptr;

AudioRecvCallback audRecvCallback;
LogCallback logCallback;
MixerCallback mixerCallback;
RoomCallback roomCallback;
RecorderCallback recorderCallback;
