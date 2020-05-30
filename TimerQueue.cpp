
#include "EventLoop.h"
#include <vector>
#include <sys/timerfd.h>
#include <algorithm>
#include <unistd.h>
#include "TimerQueue.h"
#include "Channel.h"
#include "Timer.h"
#include "muduo/base/Logging.h"

TimerQueue::TimerQueue(EventLoop *loop) : m_loop(loop), m_timerfd(createTimerfd()), m_timerChannel(new Channel(loop, m_timerfd))
{
    m_timerChannel->setReadCallback(std::bind(&TimerQueue::handleRead, this, std::placeholders::_1));
    m_timerChannel->enableReading();
}

TimerQueue::~TimerQueue()
{
    m_timerChannel->disableAll();
    m_timerChannel->remove();
    ::close(m_timerfd);
    // do not remove channel, since we're in EventLoop::dtor();
    for (const Entry &timer : m_timerSet)
    {
        delete timer.second;
    }
}

int TimerQueue::addTimer(Timer::TimerCb cb, Timestamp expiration, double interval)
{
    // Timestamp now = Timestamp::now();

    Timer *timer = new Timer(expiration, std::move(cb), interval);
    m_loop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));

    return 0;
}

void TimerQueue::addTimerInLoop(Timer *timer)
{
    bool exchangeFirstTimer = insert(timer);
    if (exchangeFirstTimer)
    {
        resetTimerfd(timer->expireation());
        LOG_WARN << "resetTimerfd done";
    }
}

struct timespec TimerQueue::howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
        microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
        (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void TimerQueue::resetTimerfd(Timestamp expiration)
{
    itimerspec newtime;
    memset(&newtime, 0, sizeof newtime);
    // newtime.it_interval.tv_sec = interval;
    newtime.it_value = howMuchTimeFromNow(expiration);
    LOG_TRACE
        << "howmuchtime:" << newtime.it_value.tv_sec;
    timerfd_settime(m_timerfd, 0, &newtime, NULL);
}

void TimerQueue::resetTimerList(std::vector<Entry> &expiredTimer)
{
    Timestamp nextexpire;
    for (const Entry &it : expiredTimer)
    {
        if (it.second->repeat())
        {
            it.second->restart();
            insert(it.second);
        }
        else
        {
            delete it.second;
        }
    }
    if (!m_timerSet.empty())
    {
        nextexpire = m_timerSet.begin()->second->expireation();
    }

    if (nextexpire.valid())
    {
        resetTimerfd(nextexpire);
    }
}
std::vector<TimerQueue::Entry> TimerQueue::getExpired()
{
    std::vector<Entry> expiredTimers;

    Entry setEntry(Timestamp::now(), reinterpret_cast<Timer *>(UINTPTR_MAX));
    auto end = m_timerSet.lower_bound(setEntry);
    std::copy(m_timerSet.begin(), end, back_inserter(expiredTimers));
    m_timerSet.erase(m_timerSet.begin(), end);

    return expiredTimers;
}

void TimerQueue::LogTimeSet(std::string &string)
{
    LOG_WARN << string;
    for (const Entry &entry : m_timerSet)
    {
        LOG_WARN << entry.first.toFormattedString() << ":" << entry.second;
    }
}
int TimerQueue::createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                   TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }
    return timerfd;
}

void TimerQueue::handleRead(int timerfd)
{
    LOG_TRACE << "handleread timerfd event";

    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at ";
    // << now.toString();
    if (n != sizeof howmany)
    {
        LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }

    std::vector<Entry> expierdTimers = getExpired();
    LOG_TRACE << "getExpired expiredvec size:" << expierdTimers.size();
    for (auto &time : expierdTimers)
    {
        time.second->run();
    }

    resetTimerList(expierdTimers);
}

bool TimerQueue::insert(Timer *timer)
{
    LOG_TRACE << "insert a timerrntry";
    bool firtTimer = false;
    auto it = m_timerSet.begin();
    Timestamp when = timer->expireation();
    if (it == m_timerSet.end() || when < it->first)
    {
        firtTimer = true;
    }

    m_timerSet.insert(std::move(Entry(when, timer)));

    return firtTimer;
}