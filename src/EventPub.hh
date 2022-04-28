#ifndef __sipxtrtc_EventPub_h__
#define __sipxtrtc_EventPub_h__ 1

#include <string>

class EventPub {
public:
  EventPub(const std::string &path);
  ~EventPub();

  int open();
  void close();

  int getFd();

  void pub(const std::string& msg);

private:
  std::string _path;
  int _fd=-1;
};

#endif