#ifndef __TIMERQUEUE_H__
#define __TIMERQUEUE_H__
#include <set>
#include <memory>
#include <functional>
#include <utility>
#include <string>
#include "muduo/base/Timestamp.h"
#include "Timer.h"
class Channel;
class EventLoop;

using muduo::Timestamp;

class TimerQueue
{

public:
    TimerQueue(EventLoop *loop);
    ~TimerQueue();

    int addTimer(Timer::TimerCb cb, Timestamp expierd, double interval);

private:
    void addTimerInLoop(Timer *timer);
    typedef std::pair<Timestamp, Timer *> Entry;
    int createTimerfd();
    void handleRead(Timestamp now);

    bool insert(Timer *timer);
    struct timespec howMuchTimeFromNow(Timestamp when);
    void resetTimerfd(Timestamp expiration);
    void resetTimerList(std::vector<Entry> &expiredTimer);
    std::vector<Entry> getExpired();

private:
    void LogTimeSet(std::string &string);

private:
    EventLoop *m_loop;
    const int m_timerfd;

    std::unique_ptr<Channel> m_timerChannel;

    std::set<Entry> m_timerSet;
};

#endif // __TIMERQUEUE_H__