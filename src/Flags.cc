#include "Flags.hh"

#include <cstdint>
#include <cstring>

#include <TRTCCloud.h>

static bool non_empty_string(const char *flagname, const std::string &value);
static bool non_zero_uint32(const char *flagname, uint32_t value);
static bool non_negative_int64(const char *flagname, int64_t value);

static bool valid_sdk_log_level(const char *flagname, int32_t value);

DEFINE_uint32(sdk_app_id, 0,
              "应用标识 - [必填] - 腾讯视频云基于 sdkAppId 完成计费统计");

DEFINE_uint32(
    room_id, 0,
    "房间号码 - [必填] - "
    "指定房间号，在同一个房间里的用户（userId）可以彼此看到对方并进行视频通话");

DEFINE_string(user_id, "",
              "用户标识 - [必填] - 当前用户的userid，相当于用户名，UTF-8编码");
DEFINE_validator(user_id, &non_empty_string);

DEFINE_string(user_sig, "",
              "用户签名 - [必填] - 当前userId对应的验证签名，相当于登录密码");
DEFINE_validator(user_sig, &non_empty_string);

DEFINE_bool(sdk_console, false, "启用或禁用控制台日志打印");

DEFINE_int32(sdk_log_level, -1,
             "0=none, 1=fatal,... 5=debug, 6=verbose; -1=SDK默认级别");

DEFINE_string(aud_playback_path, "",
              "向该文件放音(Signed 16-bit PCM, Mono, 48k, 20ms)");
DEFINE_validator(aud_playback_path, &non_empty_string);

DEFINE_string(aud_capture_path, "",
              "从该文件拾音(Signed 16-bit PCM, Mono, 48k, 20ms)");
DEFINE_validator(aud_capture_path, &non_empty_string);

bool non_empty_string(const char *flagname, const std::string &value) {
  return !value.empty();
}

bool non_zero_uint32(const char *flagname, uint32_t value) { return value > 0; }

bool non_negative_int64(const char *flagname, int64_t value) {
  return !(value < 0);
}

bool valid_sdk_log_level(const char *flagname, int32_t value) {
  return ((value >= TRTCLogLevelVerbos) ||
          (TRTCLogLevelVerbos <= TRTCLogLevelNone) || (value < 0));
}