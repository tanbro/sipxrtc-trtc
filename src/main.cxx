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

#include <TRTCCloud.h>

#include "MixerCallback.hxx"
#include "TrtcCallback.hxx"
#include "def.hxx"

using namespace std;

static bool running = true;

static void sig_int_handler(int dummy) { running = false; }
static bool is_file(const char *);

static void cap_bridge_worker();

int aud_read_sockfd = -1;
thread aud_read_trd;
mutex aud_read_mtx;
condition_variable aud_read_cv;

void aud_read(const char *path) {
  if (is_file(path)) {
    if (unlink(path)) {
      fprintf(stderr, "FATAL! unlink() error %d: %s", errno, strerror(errno));
    }
  }

  sockaddr_un recv_addr;
  memset(&recv_addr, 0, sizeof(sockaddr_un));
  recv_addr.sun_family = AF_LOCAL;
  strncpy(recv_addr.sun_path, path, sizeof(recv_addr.sun_path) - 1);

  aud_read_sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (aud_read_sockfd == -1) {
    fprintf(stderr, "FATAL! socket() returns %d: %s", errno, strerror(errno));
  }

  if (bind(aud_read_sockfd, (const sockaddr *)(&recv_addr),
           sizeof(recv_addr))) {
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
  aud_read_cv.notify_all();

  // 死循环接收
  while (1) {
    ssize_t length = recv(aud_read_sockfd, buffer, bytes_per_frame, 0);
    if (length == -1) {
      fprintf(stderr, "FATAL! recv() error %d: %s", errno, strerror(errno));
      break;
    }
    assert(length == bytes_per_frame);
    //
    // printf("收到 Audio 数据 %ld bytes\n", length);
    af->data = buffer;
    af->length = length;
    int trtc_err = trtc->sendCustomAudioData(af);
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

  trtc = createInstance(TRTC_APP_ID);
  TrtcCallback *trtcCallback = new TrtcCallback();
  trtc->setCallback(trtcCallback);

  MixerCallback *mixerCallback = new MixerCallback();
  mixer = createMediaMixer();
  mixer->setCallback(mixerCallback);

  string line;

  cout << "输入房间号:";
  getline(cin, line);
  unsigned room_id = atoi(line.c_str());
  {
    TRTCParams *params = new TRTCParams();
    params->sdkAppId = TRTC_APP_ID;
    params->roomId = room_id;
    params->userId = userid;
    params->userSig = usersig;
    trtc->enterRoom(*params, TRTCAppScene::TRTCAppSceneVideoCall);
  }

  cout << "如果已经进入了房间，按“回车”启动 Audio read 线程 (Enter):";
  char sua_sock_path[] = SUA_UDS_FILE;
  getline(cin, line);
  {
    cout << "启动 Audio read 线程 ..." << endl;
    std::unique_lock<mutex> lk(aud_read_mtx);
    aud_read_trd = thread(aud_read, sua_sock_path);
    aud_read_cv.wait(lk);
    cout << "启动 Audio read 线程成功." << endl;
  }
  cout << "启动 mixer" << endl;
  mixer->start(true, false);

  printf("ctrl-c 退出\n");
  signal(SIGINT, sig_int_handler);
  while (running) {
    sleep(1);
  }

  delete mixerCallback;
  delete trtcCallback;
}