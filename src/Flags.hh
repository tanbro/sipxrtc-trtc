#ifndef __sipxtrtc_Flags_h__
#define __sipxtrtc_Flags_h__ 1

#include <gflags/gflags.h>

DECLARE_uint32(sdk_app_id);
DECLARE_uint32(room_id);
DECLARE_string(user_id);
DECLARE_string(user_sig);
DECLARE_bool(sdk_console);
DECLARE_int32(sdk_log_level);

DECLARE_string(aud_capture_path);
DECLARE_string(aud_playback_path);

DECLARE_string(event_fifo);

#endif
