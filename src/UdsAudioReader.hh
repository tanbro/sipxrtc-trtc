#ifndef __sipxtrtc_UdsAudioReader_h__
#define __sipxtrtc_UdsAudioReader_h__ 1

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <string>

#include <TRTCCloud.h>

class UdsAudioReader {
private:
  int _fd = -1;
  std::string _path;
  sockaddr_un _addr;

  unsigned _channel;
  size_t _msecs_per_frame;
  size_t _bits_per_sample;
  size_t _sample_rate;

  uint8_t *_buffer = NULL;
  size_t _buffer_size = 0;

public:
  UdsAudioReader(const std::string &path, size_t msecsPerFrame = 20,
                 size_t bitsPerSample = 16, size_t sampleRate = 48000,
                 unsigned channel = 1);
  ~UdsAudioReader();

  int getFd();

  int open();
  void close();

  void runOnce();
};

#endif
