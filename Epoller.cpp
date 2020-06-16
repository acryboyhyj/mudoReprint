#include "Epoller.h"

#include <assert.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <sstream>

#include "Channel.h"
#include "muduo/base/Logging.h"
using namespace muduo;

namespace
{
  const int kNew = -1;
  const int kAdded = 1;
  const int kDeleted = 2;
} // namespace

Epoller::Epoller()
    : m_epollfd(::epoll_create1(EPOLL_CLOEXEC)),
      m_events(kInitEventListSize),
      m_listenfd(-1)
{
  if (m_epollfd < 0)
  {
    LOG_SYSFATAL << "Epoller::Epoller";
  }
}

Epoller::~Epoller() { ::close(m_epollfd); }

Timestamp Epoller::poll(std::vector<Channel *> &activeChannel, int timeoutMs)
{
  int numEvents = ::epoll_wait(m_epollfd, &*m_events.begin(),
                               static_cast<int>(m_events.size()), timeoutMs);
  int savedErrno = errno;
  Timestamp now(Timestamp::now());
  if (numEvents > 0)
  {
    LOG_TRACE << numEvents << " events happened";
    fillActiveChannels(activeChannel, numEvents);
    if (implicit_cast<size_t>(numEvents) == m_events.size())
    {
      m_events.resize(m_events.size() * 2);
    }
  }
  else if (numEvents == 0)
  {
    LOG_TRACE << "nothing happened";
  }
  else
  {
    // error happens, log uncommon ones
    if (savedErrno != EINTR)
    {
      errno = savedErrno;
      LOG_SYSERR << "Epoller::poll()";
    }
  }

  return now;
}
void Epoller::fillActiveChannels(std::vector<Channel *> &activeChannel,
                                 int numEvents)
{
  assert(implicit_cast<size_t>(numEvents) <= m_events.size());
  for (int i = 0; i < numEvents; ++i)
  {
    m_channel = static_cast<Channel *>(m_events[i].data.ptr);
    m_channel->setRevent(m_events[i].events);

    activeChannel.push_back(m_channel);
  }
}
const char *Epoller::operationToString(int op)
{
  switch (op)
  {
  case EPOLL_CTL_ADD:
    return "ADD";
  case EPOLL_CTL_DEL:
    return "DEL";
  case EPOLL_CTL_MOD:
    return "MOD";
  default:
    assert(false && "ERROR op");
    return "Unknown Operation";
  }
}
void Epoller::updateEvent(int operation, Channel *channel)
{
  struct epoll_event event;
  memZero(&event, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  LOG_WARN << "epoll_ctl op = " << operationToString(operation)
           << " fd = " << channel->fd() << " event = { "
           << eventsToString(channel->fd(), event.events) << " }";
  if (::epoll_ctl(m_epollfd, operation, channel->fd(), &event) < 0)
  {
    if (operation == EPOLL_CTL_DEL)
    {
      LOG_SYSERR << "epoll_ctl op =" << operationToString(operation)
                 << " fd =" << channel->fd();
    }
    else
    {
      LOG_SYSERR << "epoll_ctl op =" << operationToString(operation)
                 << " fd =" << channel->fd();
    }
  }
}
string Epoller::eventsToString(int fd, int ev)
{
  std::ostringstream oss;
  oss << fd << ": ";
  if (ev & EPOLLIN)
    oss << "IN ";
  if (ev & EPOLLPRI)
    oss << "PRI ";
  if (ev & EPOLLOUT)
    oss << "OUT ";
  if (ev & EPOLLHUP)
    oss << "HUP ";
  if (ev & EPOLLRDHUP)
    oss << "RDHUP ";
  if (ev & EPOLLERR)
    oss << "ERR ";

  return oss.str();
}

int Epoller::updateChannel(Channel *channel)
{

  updateEvent(EPOLL_CTL_ADD, channel);
  return 0;
}
int Epoller::removeChannel(Channel *channel)
{

  updateEvent(EPOLL_CTL_DEL, channel);
  return 0;
}