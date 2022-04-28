#include <sys/poll.h>

#include <climits>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <TRTCCloud.h>

#include "Flags.hh"
#include "MixerCallback.hh"
#include "RoomCallback.hh"
#include "global.hh"
#include "utils.hh"
#include "version.hh"

using namespace std;

static int hand_sigs[] = {SIGINT, SIGTERM};
static bool is_call_made = false;

static void exec_cmd(const string &);
static char cmd_buf[MAX_INPUT];

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
               << " " << ossArgs.str() << endl
               << "version " << getVersionString() << endl
               << "^^^^^^^^^^^^^^^^ startup ^^^^^^^^^^^^^^^^" << endl;

  //
  udsReader = new UdsReader(FLAGS_aud_capture_path);
  udsWriter = new UdsWriter(FLAGS_aud_playback_path);
  if (!FLAGS_event_fifo.empty()) {
    eventPub = new EventPub(FLAGS_event_fifo);
    eventPub->open();
  }

  setLogCallback(&sdkLogger);
  if (!(FLAGS_sdk_log_level < 0)) {
    setLogLevel((TRTCLogLevel)(TRTCLogLevelNone - FLAGS_sdk_log_level));
  }
  setConsoleEnabled(FLAGS_sdk_console);

  {
    lock_guard<mutex> lk(app_mtx);

    TRTCParams roomParams;
    roomParams.sdkAppId = FLAGS_sdk_app_id;
    roomParams.roomId = FLAGS_room_id;
    roomParams.userId = FLAGS_user_id;
    roomParams.userSig = FLAGS_user_sig;
    // 主播角色，即可以发送本地音视频到远端，也可以接收远端的音视频到本地
    roomParams.clientRole = TRTCClientRole::TRTCClientRole_Anchor;

    LOG(INFO) << "create room instance";
    room = createInstance(FLAGS_sdk_app_id);
    room->setCallback(&roomCallback);

    LOG(INFO) << "create mixer instance";
    mixer = createMediaMixer();
    mixer->setCallback(&mixerCallback);
    if (mixer != nullptr) {
      CHECK_EQ(0, mixer->start(true, false));
    }
    LOG(INFO) << "enter room";
    room->enterRoom(roomParams, TRTCAppScene::TRTCAppSceneVideoCall);
  }

  LOG(INFO) << "set signal handlers";
  for (int i = 0; i < (sizeof(hand_sigs) / sizeof(hand_sigs[0])); ++i) {
    PCHECK(SIG_ERR != signal(hand_sigs[i], sig_handler));
  }

  DLOG(INFO) << "polling start";
  // 一个是从 stdin 读取命令
  // 另一个是从 UDS 读取语音流
  pollfd fds[2];
  memset(&fds, 0, sizeof(fds));
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;
  int rc, nfds;
  while (!interrupted) {
    nfds = 1;
    {
      lock_guard<mutex> lk(app_mtx);
      int fd = udsReader->getFd();
      if (!(fd < 0)) {
        fds[1].fd = fd;
        fds[1].events = POLLIN;
        ++nfds;
      }
    }
    rc = poll(fds, nfds, 1000);
    if (rc > 0) {
      for (int i = 0; i < rc; ++i) {
        if (fds[i].fd == STDIN_FILENO) {
          if (fds[i].revents & POLLIN) {
            // 处理 stdin 输入的控制命令！
            memset(cmd_buf, 0, sizeof(cmd_buf));
            ssize_t nbytes;
            CHECK_ERR(nbytes = read(fds[i].fd, cmd_buf, sizeof(cmd_buf) - 1));
            CHECK_LT(nbytes, sizeof(cmd_buf));
            exec_cmd(string(cmd_buf));
          }
        } else {
          lock_guard<mutex> lk(app_mtx);
          int fd = udsReader->getFd();
          if ((fd == fds[i].fd) && (fds[i].revents & POLLIN)) {
            udsReader->read();
          }
        }
      }
    } else if (rc < 0) {
      switch (errno) {
      case EINTR:
        // 系统中断错误
        LOG(ERROR) << "errno (" << errno << "): " << strerror(errno);
        break;
      default:
        PCHECK(errno) << ": poll() failed";
        break;
      }
    }
  }
  DLOG(INFO) << "polling stopped";

  if (mixer != nullptr) {
    LOG(INFO) << "stop and destory mixer";
    mixer->stop();
    destroyMediaMixer(mixer);
  }
  if (room != nullptr) {
    LOG(INFO) << "destory room";
    {
      lock_guard<mutex> lk(app_mtx);
      if (roomCallback.getEntered()) {
        room->exitRoom();
      }
    }
    room->setCallback(nullptr);
    destroyInstance(room);
  }

  //
  if (udsReader)
    delete udsReader;
  if (udsWriter)
    delete udsWriter;
  if (eventPub)
    delete eventPub;
}

static char custmsgbuff[1000];

void exec_cmd(const string &cmd) {
  string cmd_ = trimStr(cmd);
  if (cmd_ == "sub") {
    // 订阅所有的远端声音
    roomCallback.suball();
    if (udsReader->getFd() < 0)
      udsReader->open();
    if (udsWriter->getFd() < 0)
      udsWriter->open();
  } else if (cmd_ == "unsub") {
    // 取消订阅所有的远端声音
    roomCallback.unsuball();
    if (udsReader->getFd() >= 0)
      udsReader->close();
    if (udsWriter->getFd() >= 0)
      udsWriter->close();
  } else if (cmd_.substr(0, 4) == "msg ") {
    string msg = trimStr(cmd_.substr(4, cmd_.length() - 4));
    CHECK_GT(sizeof(custmsgbuff), msg.length());
    memset(custmsgbuff, 0, sizeof(custmsgbuff));
    strncpy(custmsgbuff, msg.c_str(), sizeof(custmsgbuff) - 1);
    LOG(INFO) << "sendCustomCmdMsg: " << custmsgbuff;
    LOG_IF(ERROR, room->sendCustomCmdMsg(1, (const unsigned char *)custmsgbuff,
                                         strlen(custmsgbuff), true, true))
        << "sendCustomCmdMsg failed";
  }
}
