#ifndef __Channel_H__
#define __Channel_H__

#include <functional>
#include <string>
#include <sys/epoll.h>
#include "muduo/base/Timestamp.h"
using muduo::Timestamp;
class EventLoop;
class Channel
{
  typedef std::function<void()> EventCallback;
  typedef std::function<void(Timestamp readTime)> REventCallback;

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
    m_events &= ~EPOLLIN;
    remove();
  }
  bool isWriting()
  {
    return m_events & EPOLLOUT;
  }
  void enableWrite()
  {
    m_events |= EPOLLOUT;
    update();
  }
  void disableWrite()
  {
    m_events &= ~EPOLLOUT;
    update();
  }

  void disableAll()
  {
    m_events = 0;
    remove();
  }

  void setReadCallback(REventCallback cb) { m_readCallback = std::move(cb); }
  void setWriteCallback(EventCallback cb) { m_writeCallback = std::move(cb); }
  void setCloseCallback(EventCallback cb) { m_closeCallback = std::move(cb); }
  void setErrorCallback(EventCallback cb) { m_errorCallback = std::move(cb); }

  void setRevent(int event);
  void handleEvent(Timestamp pollReturnTime);
  int fd() { return m_fd; };
  int events() { return m_events; }
  EventLoop *ownLoop() { return m_eventLoop; }
  void remove();

  bool isNoneEvent() const { return m_events == kNoneEvent; }
  int index() { return m_index; }
  void set_index(int idx) { m_index = idx; }

private:
  void update();

  int m_fd;
  int m_index;
  EventLoop *m_eventLoop;
  int m_events;
  int m_revent;

  REventCallback m_readCallback;
  EventCallback m_writeCallback;
  EventCallback m_closeCallback;
  EventCallback m_errorCallback;

  static const int kNoneEvent = 0;
};

#endif