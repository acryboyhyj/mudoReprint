
#include "EventLoopThreadPoll.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
EventLoopThreadPoll::EventLoopThreadPoll(EventLoop *loop, const std::string &name) : m_baseLoop(loop),
                                                                                     m_threadNum(0),
                                                                                     m_name(name),
                                                                                     m_next(0),
                                                                                     m_started(false)
{
}

EventLoopThreadPoll::~EventLoopThreadPoll()
{
}

void EventLoopThreadPoll::start(std::function<void(EventLoop *)> threadInitCallBack)
{
    m_baseLoop->assertInLoopThread();
    assert(!m_started);
    m_started = true;
    for (int i = 0; i < m_threadNum; i++)
    {
        char buf[m_name.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", m_name.c_str(), i);
        EventLoopThread *t = new EventLoopThread(buf, threadInitCallBack);

        m_threads.push_back(std::unique_ptr<EventLoopThread>(t));
        m_loops.push_back(t->startLoop());
    }
}

EventLoop *EventLoopThreadPoll::getNextLoop()
{
    m_baseLoop->assertInLoopThread();
    LOG_INFO << "cccccccccccccccccccccccccccc";
    assert(m_started);
    if (m_loops.size() == 0)
    {
        return m_baseLoop;
    }
    else
    {
        EventLoop *loop = m_loops[m_next++];
        if (m_next >= static_cast<int>(m_loops.size()))
        {
            m_next = 0;
        }
        return loop;
    }
}