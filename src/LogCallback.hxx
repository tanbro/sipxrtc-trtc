#ifndef __sipxtrtc_LogCallback_h__
#define __sipxtrtc_LogCallback_h__ 1

#include <stdexcept>
#include <string>

#include "TRTCCloud.h"

class LogCallback : public ITRTCLogCallback {
public:
  void onLog(const char *log, TRTCLogLevel level, const char *module);

  const std::string &getLevelName(TRTCLogLevel level);

private:
  static const std::string DEBUG;
  static const std::string INFO;
  static const std::string WARN;
  static const std::string ERROR;
  static const std::string FATAL;
};

#endif