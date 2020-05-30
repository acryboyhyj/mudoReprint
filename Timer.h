#ifndef __TIMER_H__
#define __TIMER_H__
#include <functional>
#include "muduo/base/Timestamp.h"
#include "muduo/base/Logging.h"
using muduo::Timestamp;
class Timer
{

public:
    typedef std::function<void()> TimerCb;
    Timer(Timestamp expiration, TimerCb cb, double interval) : m_expiration(expiration), m_cb(cb), m_interval(interval)
    {
    }
    ~Timer() { LOG_WARN << "~Timer"; }
    void run()
    {
        if (m_cb)
        {
            // m_cb(Timestamp::now());
            m_cb();
        }
    }

    bool repeat()
    {
        return m_interval > 0.0;
    }
    double interval()
    {
        return m_interval;
    }

    Timestamp expireation() { return m_expiration; };

    void restart()
    {
        if (repeat())
        {
            m_expiration = addTime(m_expiration, m_interval);
        }
        else
        {
            m_expiration = Timestamp::invalid();
        }
    }

private:
    Timestamp m_expiration;
    TimerCb m_cb;
    double m_interval;
};

#endif // __TIMER_H__