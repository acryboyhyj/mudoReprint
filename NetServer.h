#ifndef __NETSERVER_H__
#define __NETSERVER_H__
#include <memory>
#include <thread>

class EventLoop;
class Channel;
class NetServer
{
public:
  explicit NetServer(EventLoop *eventLoop);
  ~NetServer();
  int createListen();
  void readThread(int fd);
  NetServer(const NetServer &) = delete;
  NetServer &operator=(const NetServer &) = delete;
  void readCb(int fd);
  void start();

private:
  void net();

private:
  EventLoop *m_loop;
  Channel *m_channel;

  std::unique_ptr<std::thread> m_netsvr;
  std::unique_ptr<std::thread> m_readsvr;
  int m_listenfd;

  int m_a = 0;
};
#endif // __NETSERVER_H__