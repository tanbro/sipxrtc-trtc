#ifndef __sipxtrtc_SdkLogger_h__
#define __sipxtrtc_SdkLogger_h__ 1

#include <stdexcept>
#include <string>

#include <TRTCCloud.h>

class SdkLogger : public ITRTCLogCallback {
public:
  void onLog(const char *log, TRTCLogLevel level, const char *module);
};

#endif