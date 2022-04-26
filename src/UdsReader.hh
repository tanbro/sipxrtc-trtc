#ifndef __sipxsua_UdsReader_h__
#define __sipxsua_UdsReader_h__ 1

#include <sys/un.h>

#include <cstddef>
#include <string>

#include <TRTCCloud.h>

#include "UdsBase.hh"

class UdsReader : public UdsBase {
public:
  UdsReader(const std::string &path);

  virtual ssize_t read();

  virtual int open() override;
  virtual void close() override;

protected:
  sockaddr_un addr;
  std::string path;

private:
  TRTCAudioFrame trtcAudFrm;
  uint8_t read_buffer[1920];
  void resetTrtcAudFrm();
};

#endif
