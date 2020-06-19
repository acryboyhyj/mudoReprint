#ifndef __EVENTLOOP_H__
#define __EVENTLOOP_H__
#include <atomic>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include "Epoller.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/base/Timestamp.h"
#include "Timer.h"
class Channel;
using muduo::Timestamp;
class TimerQueue;
class EventLoop
{

public:
    typedef std::function<void()> Functor;
    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;
    void loop();

    void quit();
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);

    void runInLoop(Functor func);
    void queueInLoop(Functor func);

    //one thread guranted
    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }
    bool isInLoopThread() const { return m_tid == muduo::CurrentThread::tid(); }
    void abortNotInLoopThread();

    //timer
    void runAt(Timestamp now, Timer::TimerCb cb);
    void runEvery(double interval, Timer::TimerCb cb);
    void runAfter(double delay, Timer::TimerCb cb);


private:
    //wakeup
    void wakeup();
    int createEventfd();
    void handleWakeUpRead();

    void doPendingFunctor();

private:
    static const int KinitSize;

private:
    std::unique_ptr<Epoller> m_epoller;

    std::unique_ptr<Channel> m_channel;
    std::vector<Channel *> m_activeChannels;
    std::unique_ptr<Channel> m_currChannel;
    std::atomic<bool> m_quit;
    const pid_t m_tid;
    std::unique_ptr<TimerQueue> m_timerQueue;

    int m_wakeUpFd;
    std::unique_ptr<Channel> m_wakeUpChannel;
    std::vector<Functor> m_doPendingFuncVec;
    std::mutex m_pendFuncVecMutex;
    bool m_callingPendingFunctors;
};
#endif // __EVENTLOOP_H__
