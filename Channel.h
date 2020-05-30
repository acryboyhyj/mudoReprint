#ifndef __Channel_H__
#define __Channel_H__

#include <functional>
#include <string>
#include <sys/epoll.h>
class EventLoop;
class Channel
{
  typedef std::function<void()> EventCallback;
  typedef std::function<void(int)> REventCallback;

public:
  Channel(EventLoop *loop, int fd);
  ~Channel();

  void enableReading()
  {
    m_events |= EPOLLIN;
    update();
  }
  void disableReading()
  {
    m_events &= !EPOLLIN;
    remove();
  }

  void disableAll()
  {
    m_events = 0;
    update();
  }

  void setReadCallback(REventCallback cb) { m_readCallback = std::move(cb); }
  void setWriteCallback(EventCallback cb) { m_writeCallback = std::move(cb); }
  void setCloseCallback(EventCallback cb) { m_closeCallback = std::move(cb); }
  void setErrorCallback(EventCallback cb) { m_errorCallback = std::move(cb); }

  void setRevent(int event);
  void handleEvent();
  int fd() { return m_fd; };
  int events() { return m_events; }
  EventLoop *ownLoop() { return m_eventLoop; }
  void remove();

private:
  void update();

  int m_fd;
  EventLoop *m_eventLoop;
  int m_events;
  int m_revent;

  REventCallback m_readCallback;
  EventCallback m_writeCallback;
  EventCallback m_closeCallback;
  EventCallback m_errorCallback;
};

#endif