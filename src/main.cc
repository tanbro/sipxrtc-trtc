#include <assert.h>
#include <errno.h>
#include <memory.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "TRTCCloud.h"

#include "LogCallback.hh"
#include "MixerCallback.hh"
#include "RoomCallback.hh"
#include "UdsAudioReader.hh"
#include "def.hh"
#include "global.hh"

using namespace std;

static bool exiting = true;

static void sig_int_handler(int dummy) { exiting = true; }

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  string userid = TRTC_USER_ID;
  string usersig = TRTC_USER_SIG;

  int errCode;
  string line;

  // setLogDirPath("logs");
  /**
   * 如果不想控制台输出SDK日志，请使用 setConsoleEnabled(false)
   */
  setConsoleEnabled(false);

  // setLogLevel(TRTCLogLevel::TRTCLogLevelWarn);
  // setLogCallback(&logCallback);

  cout << "输入房间号:";
  getline(cin, line);
  string roomName = line.c_str();
  TRTCParams params;
  params.sdkAppId = TRTC_APP_ID;
  params.roomId = stol(roomName);
  params.userId = userid;
  params.userSig = usersig;
  // 主播角色，即可以发送本地音视频到远端，也可以接收远端的音视频到本地
  params.clientRole = TRTCClientRole::TRTCClientRole_Anchor;

  {
    lock_guard<mutex> lk(trtc_app_mutex);

    room = createInstance(TRTC_APP_ID);
    room->setCallback(&roomCallback);

    cout << "启动 mixer ..." << endl;
    mixer = createMediaMixer();
    mixerCallback.open(RTC_UDS_FILE);
    mixer->setCallback(&mixerCallback);
    if (mixer != nullptr) {
      int errCode = mixer->start(true, false);
      if (errCode) {
        cerr << "!启动 mixer 失败 (" << errCode << ")" << endl;
        return -1;
      } else {
        cout << "启动 mixer 成功" << endl;
      }
    }

    room->enterRoom(params, TRTCAppScene::TRTCAppSceneVideoCall);
  }

  DVLOG(3) << "等待进入房间 ...";
  while (!roomCallback.getEntered()) {
    this_thread::sleep_for(chrono::milliseconds(100));
  }
  DVLOG(3) << "已经进入房间!";

  pollfd fds;
  nfds_t nfds = 1;
  fds.events = POLLRDNORM;

  DVLOG(1) << "打开 udsAudioReader";
  UdsAudioReader audReader(SUA_UDS_FILE);
  fds.fd = audReader.open();

  bool exiting = false;
  signal(SIGINT, sig_int_handler);
  printf("ctrl-c 退出\n");
  while (!exiting) {
    int rc;
    CHECK_ERR(rc = poll(&fds, 1, 5000));
    if (rc != 0) {
      DLOG_EVERY_N(INFO, 10) << "poll() revents: " << fds.revents;
      if (fds.revents & POLLRDNORM) {
        DLOG_EVERY_N(INFO, 10) << "audReader.runOnce() ... ";
        audReader.runOnce();
      }
    }
    fds.revents = 0;
  }

  audReader.close();

  if (mixer != nullptr) {
    mixer->stop();
    destroyMediaMixer(mixer);
  }
  if (room != nullptr) {
    room->setCallback(nullptr);
    destroyInstance(room);
  }
}