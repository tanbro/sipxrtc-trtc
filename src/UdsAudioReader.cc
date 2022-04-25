#include "UdsAudioReader.hh"

#include <memory.h>

#include <sys/socket.h>

#include <glog/logging.h>

#include "global.hh"

using namespace std;

UdsAudioReader::UdsAudioReader(const string &path, size_t msecsPerFrame,
                               size_t bitsPerSample, size_t sampleRate,
                               unsigned channel) {
  CHECK_EQ(20, msecsPerFrame);
  CHECK_EQ(1, channel);
  CHECK_EQ(16, bitsPerSample);
  CHECK_EQ(48000, sampleRate);

  _buffer_size = sampleRate * channel * msecsPerFrame / 1000 * bitsPerSample /
                 sizeof(uint8_t) / 8;
  CHECK_EQ(1920, _buffer_size);

  _path = path;

  _msecs_per_frame = msecsPerFrame;
  _bits_per_sample = bitsPerSample;
  _sample_rate = sampleRate;
  _channel = channel;

  _buffer = (uint8_t *)malloc(_buffer_size * sizeof(uint8_t));
}

UdsAudioReader::~UdsAudioReader() {
  close();
  if (_buffer) {
    free(_buffer);
  }
}

int UdsAudioReader::getFd() { return _fd; }

int UdsAudioReader::open() {
  CHECK_GT(0, _fd);

  struct stat statbuf;
  int stat_ret = stat(_path.c_str(), &statbuf);
  if (!stat_ret) {
    DVLOG(1) << "unlink(\"" << _path << "\")";
    CHECK_ERR(unlink(_path.c_str()));
  }

  memset(&_addr, 0, sizeof(sockaddr_un));
  _addr.sun_family = AF_LOCAL;
  strncpy(_addr.sun_path, _path.c_str(), sizeof(_addr.sun_path) - 1);

  CHECK_ERR(_fd = socket(AF_LOCAL, SOCK_DGRAM, 0));
  CHECK_ERR(bind(_fd, (const sockaddr *)(&_addr), sizeof(_addr)));

  DVLOG(1) << "bind " << _fd << ":" << _path;

  return _fd;
}

void UdsAudioReader::close() {
  if (_fd < 0) {
    return;
  }
  if (_fd >= 0) {
    DVLOG(1) << "close " << _fd;
    PCHECK(::close(_fd));
    _fd = -1;
  }
}

void UdsAudioReader::runOnce() {
  ssize_t n_bytes = recv(_fd, _buffer, _buffer_size, 0);
  VLOG_IF_EVERY_N(3, n_bytes > 0, 100) << "recv() -> " << n_bytes << " bytes";
  if (n_bytes < 0) {
    if (errno != EWOULDBLOCK) {
      PCHECK(errno) << ": recv() failed: ";
    }
    return;
  }

  CHECK_EQ(_buffer_size, n_bytes);

  TRTCAudioFrame af;
  af.audioFormat = TRTCAudioFrameFormat::TRTCAudioFrameFormat_PCM;
  af.data = _buffer;
  af.length = _buffer_size;
  af.sampleRate = 48000;
  af.channel = 1;
  af.timestamp = 0;

  CHECK_EQ(0, room->sendCustomAudioData(&af));
}
