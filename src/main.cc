#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

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

#include "LogCallback.hh"
#include "MixerCallback.hh"
#include "RoomCallback.hh"

#include "def.hh"
#include "global.hh"

using namespace std;

static bool exiting = false;

static void sig_int_handler(int dummy) { exiting = true; }

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  VLOG(1) << "[" << this_thread::get_id() << "]"
          << "启动了！！！！！！！";

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

  udsReader = new UdsReader(SUA_UDS_FILE);
  udsReader->open();
  udsWriter = new UdsWriter(RTC_UDS_FILE);
  udsWriter->open();

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
    lock_guard<mutex> lk(app_mtx);

    room = createInstance(TRTC_APP_ID);
    room->setCallback(&roomCallback);

    cout << "启动 mixer ..." << endl;
    mixer = createMediaMixer();
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

  signal(SIGINT, sig_int_handler);
  printf("ctrl-c 退出\n");

  DLOG(INFO) << "启动 poll ...";

  pollfd fds;
  int rc;
  // cout << "xxx:";
  // getline(cin, line);
  while (!exiting) {
    memset(&fds, 0, sizeof(pollfd));
    fds.fd = udsReader->getFd();
    fds.events = POLLIN;
    CHECK_ERR(rc = poll(&fds, 1, 1000));
    if (!rc) { // timeout
      this_thread::sleep_for(chrono::milliseconds(100));
      continue;
    }
    if (fds.revents & POLLERR) {
      LOG(WARNING) << "POLLERR";
    }
    if (fds.revents & POLLHUP) {
      LOG(WARNING) << "POLLHUP";
    }
    if (fds.revents & POLLNVAL) {
      LOG(WARNING) << "POLLNVAL";
    }
    if (fds.revents & POLLIN) {
      udsReader->read();
    }
  }

  if (mixer != nullptr) {
    mixer->stop();
    destroyMediaMixer(mixer);
  }
  if (room != nullptr) {
    room->setCallback(nullptr);
    destroyInstance(room);
  }

  //
  if (udsReader)
    delete udsReader;
  if (udsWriter)
    delete udsWriter;
}