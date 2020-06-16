#ifndef __EPOLL_H__
#define __EPOLL_H__
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "Channel.h"
using std::string;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
class Epoller
{
public:
  Epoller();
  ~Epoller();

  int updateChannel(Channel *channel);
  int removeChannel(Channel *channel);
  Timestamp poll(std::vector<Channel *> &activeChannels, int timeoutMs);
  void fillActiveChannels(std::vector<Channel *> &activeChannels, int numEvents);

  string eventsToString(int fd, int ev);
  static const char *operationToString(int op);

private:
  static const int kInitEventListSize = 16;
  void updateEvent(int operation, Channel *channel);
  // void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
  // void update(int operation, Channel* channel);

  typedef std::vector<struct epoll_event> EventList;

  int m_epollfd;
  EventList m_events;
  int m_listenfd;

  Channel *m_channel;
};

#endif // __EPOLL_H__