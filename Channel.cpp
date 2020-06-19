#include "Channel.h"

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include "EventLoop.h"
#include "muduo/base/Logging.h"
#include "EventLoop.h"
Channel::Channel(EventLoop *loop, int fd)
    : m_fd(fd),
      m_index(-1),
      m_eventLoop(loop),
      m_events(0),
      m_revent(0)

{
}

Channel::~Channel()
{
}
void Channel::handleEvent(Timestamp pollReturnTime)
{
    int revent = m_revent;
    if ((revent & EPOLLHUP) && !(revent & EPOLLIN))
    {
        if (true)
        {
            //   LOG_WARN << "fd = " << rrevent.data.fd
            //            << " Channel::handle_event() EPOLLHUP";
        }
        if (m_closeCallback)
            m_closeCallback();
    }

    if (revent & (EPOLLERR))
    {
        if (m_errorCallback)
            m_closeCallback();
    }
    if (revent & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        if (m_readCallback)
            m_readCallback(pollReturnTime);
    }
    if (revent & EPOLLOUT)
    {
        if (m_writeCallback)
            m_writeCallback();
    }
}

void Channel::update()
{
    m_eventLoop->assertInLoopThread();

    m_eventLoop->updateChannel(this);
}

void Channel::remove()
{
    m_eventLoop->assertInLoopThread();

    m_eventLoop->removeChannel(this);
}
void Channel::setRevent(int event)
{
    m_revent = event;
}
