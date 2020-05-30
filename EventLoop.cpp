#include "EventLoop.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <vector>
#include "Channel.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Timestamp.h"
#include "assert.h"
#include "TimerQueue.h"
#include "time.h"

const int EventLoop::KinitSize = 16;

thread_local EventLoop *t_eventLoopTid = 0;
EventLoop::EventLoop()
    : m_epoller(new Epoller()),
      m_channel(nullptr),
      m_activeChannels(KinitSize),
      m_currChannel(nullptr),
      m_quit(false),
      m_tid(muduo::CurrentThread::tid()),
      m_timerQueue(new TimerQueue(this)),
      m_wakeUpFd(createEventfd()),
      m_wakeUpChannel(new Channel(this, m_wakeUpFd)),
      m_callingPendingFunctors(false)

{
    if (t_eventLoopTid)
    {
        LOG_FATAL << "Another EventLoop " << t_eventLoopTid
                  << " exists in this thread " << m_tid;
    }
    else
    {
        t_eventLoopTid = this;
    }
    m_wakeUpChannel->setReadCallback(std::bind(&EventLoop::handleWakeUpRead, this));
    m_wakeUpChannel->enableReading();
}

EventLoop::~EventLoop()
{
    LOG_WARN << "EventLoop " << this << " of thread " << m_tid
             << " destructs in thread " << muduo::CurrentThread::tid();
    // m_wakeUpChannel->disableAll();

    m_wakeUpChannel->remove();

    ::close(m_wakeUpFd);
    t_eventLoopTid = NULL;
}

void EventLoop::loop()
{
    assertInLoopThread();

    while (!m_quit.load(std::memory_order_relaxed))
    {
        LOG_WARN << "loop";
        m_activeChannels.clear();

        m_epoller->poll(m_activeChannels, 10000);

        for (const auto &c : m_activeChannels)
        {
            c->handleEvent();
        }

        doPendingFunctor();
    }
}

void EventLoop::quit()
{
    LOG_WARN << "quitLoop";
    m_quit.store(true, std::memory_order_relaxed);
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::updateChannel(Channel *channel)
{
    assert(channel->ownLoop() == this);
    m_epoller->updateChannel(EPOLL_CTL_ADD, channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    assert(channel->ownLoop() == this);
    m_epoller->updateChannel(EPOLL_CTL_DEL, channel);
}

void EventLoop::runInLoop(Functor func)
{
    if (isInLoopThread())
    {
        func();
    }
    else
    {
        queueInLoop(std::move(func));
    }
}
void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << m_tid
              << ", current thread id = " << muduo::CurrentThread::tid();
}

void EventLoop::runAt(Timestamp time, Timer::TimerCb cb)
{
    m_timerQueue->addTimer(std::move(cb), time, 0);
}

void EventLoop::runEvery(double interval, Timer::TimerCb cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    m_timerQueue->addTimer(std::move(cb), time, interval);
}

void EventLoop::runAfter(double delay, Timer::TimerCb cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, std::move(cb));
}
void EventLoop::queueInLoop(Functor cb)
{
    LOG_WARN << "quueInLoop";
    {
        std::lock_guard<std::mutex> lock(m_pendFuncVecMutex);
        m_doPendingFuncVec.push_back(std::move(cb));
    }

    if (!isInLoopThread() || m_callingPendingFunctors)
    {
        wakeup();
    }
}

void EventLoop::wakeup()
{
    LOG_WARN << "EventLoop::wakeup() writes ";
    uint64_t one = 1;
    ssize_t n = write(m_wakeUpFd, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}
int EventLoop::createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

void EventLoop::handleWakeUpRead()
{
    uint64_t one = 1;
    ssize_t n = read(m_wakeUpFd, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctor()
{
    std::vector<Functor> functors;
    {
        std::lock_guard<std::mutex> lock(m_pendFuncVecMutex);
        functors.swap(m_doPendingFuncVec);
    }
    m_callingPendingFunctors = true;
    for (const Functor &fun : functors)
    {
        fun();
    }
    m_callingPendingFunctors = false;
}