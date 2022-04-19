#include "Receiver.hxx"

#include <string>

#include "global.hxx"

Receiver::Receiver() {
  // sendto_addr = (sockaddr_un *)calloc(1, sizeof(sockaddr_un));
  // sendto_addr->sun_family = AF_UNIX;
  // strncpy(sendto_addr->sun_path, path.c_str(),
  //         sizeof(sendto_addr->sun_path) - 1);

  // sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
  // if (sockfd == -1) {
  //   perror(strerror(errno));
  // }
}

Receiver::~Receiver() {
  // if (sockfd != -1) {
  //   close(sockfd);
  // }
  // if (sendto_addr) {
  //   free(sendto_addr);
  // }
}

void Receiver::onRecvAudioFrame(const char *userId, TRTCAudioFrame *frame) {
  mixer->addAudioFrame(userId, frame);
  // ssize_t sent_bytes =
  //     sendto(sockfd, frame->data, frame->length, 0,
  //            (struct sockaddr *)sendto_addr, sizeof(*sendto_addr));
  // if (sent_bytes < 0) {
  //   switch (errno) {
  //   case ENOENT:
  //     break;
  //   case ECONNREFUSED:
  //     break;
  //   default: {
  //     perror(strerror(errno));
  //   } break;
  //   }
  // }
}
