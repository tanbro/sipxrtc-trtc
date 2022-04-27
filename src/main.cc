#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <TRTCCloud.h>

#include "MixerCallback.hh"
#include "RoomCallback.hh"

#include "Flags.hh"
#include "def.hh"
#include "global.hh"
#include "version.hh"

using namespace std;

static int hand_sigs[] = {SIGINT, SIGTERM};
static bool is_call_made = false;

static void sig_handler(int sig) {
  LOG(WARNING) << "signal: 0x" << hex << sig;
  for (int i = 0; i < (sizeof(hand_sigs) / sizeof(hand_sigs[0])); ++i) {
    PCHECK(SIG_ERR != signal(hand_sigs[i], NULL));
  }
  interrupted = true;
}

int main(int argc, char *argv[]) {
  ostringstream ossArgs;
  for (int i = 0; i < argc; ++i) {
    ossArgs << argv[i] << " ";
  }

  gflags::SetVersionString(getVersionString());
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  LOG(WARNING) << endl
               << "================ startup ================" << endl
               << "[0x" << hex << this_thread::get_id() << dec << "]"
               << " " << ossArgs.str() << endl
               << "version " << getVersionString() << endl
               << "^^^^^^^^^^^^^^^^ startup ^^^^^^^^^^^^^^^^" << endl;

  setLogCallback(&sdkLogger);
  setLogLevel((TRTCLogLevel)FLAGS_sdk_log_level);
  setConsoleEnabled(FLAGS_sdk_console);

  udsReader = new UdsReader(FLAGS_aud_capture_path);
  udsReader->open();
  udsWriter = new UdsWriter(FLAGS_aud_playback_path);
  udsWriter->open();

  TRTCParams params;
  params.sdkAppId = FLAGS_sdk_app_id;
  params.roomId = FLAGS_room_id;
  params.userId = FLAGS_user_id;
  params.userSig = FLAGS_user_sig;
  // 主播角色，即可以发送本地音视频到远端，也可以接收远端的音视频到本地
  params.clientRole = TRTCClientRole::TRTCClientRole_Anchor;

  {
    lock_guard<mutex> lk(app_mtx);

    LOG(INFO) << "create room instance";
    room = createInstance(TRTC_APP_ID);
    room->setCallback(&roomCallback);

    LOG(INFO) << "create mixer instance";
    mixer = createMediaMixer();
    mixer->setCallback(&mixerCallback);
    if (mixer != nullptr) {
      CHECK_EQ(0, mixer->start(true, false));
    }
    LOG(INFO) << "enter room";
    room->enterRoom(params, TRTCAppScene::TRTCAppSceneVideoCall);
  }
  while (!roomCallback.getEntered()) {
    this_thread::sleep_for(chrono::milliseconds(100));
  }

  LOG(INFO) << "set signal handlers";
  for (int i = 0; i < (sizeof(hand_sigs) / sizeof(hand_sigs[0])); ++i) {
    PCHECK(SIG_ERR != signal(hand_sigs[i], sig_handler));
  }

  DLOG(INFO) << "start polling";
  pollfd fds;
  int rc;
  while (!interrupted) {
    memset(&fds, 0, sizeof(pollfd));
    fds.fd = udsReader->getFd();
    fds.events = POLLIN;
    rc = poll(&fds, 1, 1000);
    if (rc > 0) {
      if (fds.revents & POLLIN) {
        udsReader->read();
      }
    } else if (rc < 0) {
      // ERROR!
      if (errno == EINTR) {
        // 系统中断，不管
        LOG(ERROR) << "(" << errno << "): " << strerror(errno);
      } else {
        PCHECK(errno);
      }
    }
  }

  if (mixer != nullptr) {
    LOG(INFO) << "stop and destory mixer";
    mixer->stop();
    destroyMediaMixer(mixer);
  }
  if (room != nullptr) {
    LOG(INFO) << "destory room";
    room->setCallback(nullptr);
    destroyInstance(room);
  }

  //
  if (udsReader)
    delete udsReader;
  if (udsWriter)
    delete udsWriter;
}
