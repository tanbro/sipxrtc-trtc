#include "SdkLogger.hh"

#include <glog/logging.h>

void SdkLogger::onLog(const char *log, TRTCLogLevel level, const char *module) {
  switch (level) {
  case TRTCLogLevelDebug:
    VLOG(1) << log;
    break;
  case TRTCLogLevelInfo:
    LOG(INFO) << log;
    break;
  case TRTCLogLevelWarn:
    LOG(WARNING) << log;
    break;
  case TRTCLogLevelError:
    LOG(ERROR) << log;
    break;
  case TRTCLogLevelFatal:
    LOG(FATAL) << log;
    break;
  default:
    break;
  }
}
