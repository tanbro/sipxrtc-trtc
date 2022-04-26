#ifndef __sipxsua_UdsBase_h
#define __sipxsua_UdsBase_h 1

/**
 * @brief UDS 的读写的基础类型
 *
 */
class UdsBase {
public:
  UdsBase(){};
  virtual ~UdsBase();

  virtual int open();
  virtual void close();

  virtual int getFd();

protected:
  int fd = -1;
};

#endif
