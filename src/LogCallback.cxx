#include "LogCallback.hxx"

#include <iostream>
#include <sstream>

using namespace std;

void LogCallback::onLog(const char *log, TRTCLogLevel level,
                        const char *module) {
  cerr << "[" << getLevelName(level) << "] " << module << " - " << log << endl;
}

const string LogCallback::DEBUG = "DEBUG";
const string LogCallback::INFO = "INFO ";
const string LogCallback::WARN = "WARN ";
const string LogCallback::ERROR = "ERROR";
const string LogCallback::FATAL = "FATAL";

inline const string &LogCallback::getLevelName(TRTCLogLevel level) {
  string levelName;
  switch (level) {
  case TRTCLogLevelDebug:
    return LogCallback::DEBUG;
    break;
  case TRTCLogLevelInfo:
    return INFO;
    break;
  case TRTCLogLevelWarn:
    return WARN;
    break;
  case TRTCLogLevelError:
    return ERROR;
    break;
  case TRTCLogLevelFatal:
    return FATAL;
    break;
  default:
    break;
  }
  ostringstream oss;
  oss << "level " << level << " is out of range.";
  throw new out_of_range(oss.str());
}