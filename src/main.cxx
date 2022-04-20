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

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

#include "TRTCCloud.h"

#include "LogCallback.hxx"
#include "MixerCallback.hxx"
#include "RoomCallback.hxx"
#include "def.hxx"
#include "global.hxx"

using namespace std;

static bool exiting = true;

static void sig_int_handler(int dummy) { exiting = true; }
static bool is_file(const char *);

int fd_play_sua = -1;
thread trd_play_sua;
mutex mtx_play_sua;
condition_variable cv_play_sua;

void fn_play_sua(const char *path) {
  if (is_file(path)) {
    if (unlink(path)) {
      fprintf(stderr, "FATAL! unlink() error %d: %s", errno, strerror(errno));
    }
  }

  sockaddr_un recv_addr;
  memset(&recv_addr, 0, sizeof(sockaddr_un));
  recv_addr.sun_family = AF_LOCAL;
  strncpy(recv_addr.sun_path, path, sizeof(recv_addr.sun_path) - 1);

  fd_play_sua = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (fd_play_sua == -1) {
    fprintf(stderr, "FATAL! socket() returns %d: %s", errno, strerror(errno));
  }

  if (bind(fd_play_sua, (const sockaddr *)(&recv_addr), sizeof(recv_addr))) {
    fprintf(stderr, "FATAL! bind() error %d: %s", errno, strerror(errno));
  }

  TRTCAudioFrame *af = new TRTCAudioFrame();
  af->audioFormat = TRTCAudioFrameFormat::TRTCAudioFrameFormat_PCM;

  size_t msecs_per_frame = 20;
  size_t bits_per_sample = 16;
  size_t bytes_per_frame = af->sampleRate * af->channel * msecs_per_frame /
                           1000 * bits_per_sample / sizeof(uint8_t) / 8;
  assert(bytes_per_frame == 1920);
  uint8_t *buffer = (uint8_t *)malloc(bytes_per_frame * sizeof(uint8_t));

  // 标记启动成功
  cv_play_sua.notify_all();

  // 死循环接收
  while (1) {
    ssize_t length = recv(fd_play_sua, buffer, bytes_per_frame, 0);
    if (length == -1) {
      fprintf(stderr, "FATAL! recv() error %d: %s", errno, strerror(errno));
      break;
    }
    assert(length == bytes_per_frame);
    //
    // printf("收到 Audio 数据 %ld bytes\n", length);
    af->data = buffer;
    af->length = length;
    int trtc_err = room->sendCustomAudioData(af);
    if (trtc_err) {
      fprintf(stderr, "FATAL! sendCustomAudioData() error %d\n", trtc_err);
      break;
    }
  }

  free(af);
  free(buffer);
}

bool is_file(const char *pathname) {
  struct stat statbuf;
  int ret = stat(pathname, &statbuf);
  if (ret)
    return false;
  return true;
}

int main(int argc, char *argv[]) {

  string userid = TRTC_USER_ID;
  string usersig = TRTC_USER_SIG;

  int errCode;
  string line;

  // setLogDirPath("logs");
  /**
   * 如果不想控制台输出SDK日志，请使用 setConsoleEnabled(false)
   */
  setConsoleEnabled(false);

  setLogLevel(TRTCLogLevel::TRTCLogLevelWarn);
  setLogCallback(&logCallback);

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

  // cout << "如果已经进入了房间，按“回车”启动 SUA 放音线程 (Enter):";
  // char sua_sock_path[] = SUA_UDS_FILE;
  // getline(cin, line);
  // {
  //   cout << "启动 SUA 放音线程 ..." << endl;
  //   unique_lock<mutex> lk(mtx_play_sua);
  //   trd_play_sua = thread(fn_play_sua, sua_sock_path);
  //   cv_play_sua.wait(lk);
  //   cout << "启动 SUA 放音线程成功." << endl;
  // }

  printf("ctrl-c 退出\n");
  bool exiting = false;
  signal(SIGINT, sig_int_handler);
  while (!exiting) {
    sleep(1);
  }

  // wrapper.StopMixRecord();

  if (mixer != nullptr) {
    mixer->stop();
    destroyMediaMixer(mixer);
  }
  if (room != nullptr) {
    room->setCallback(nullptr);
    destroyInstance(room);
  }
}