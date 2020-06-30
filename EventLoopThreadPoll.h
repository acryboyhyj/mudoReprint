#ifndef __EVENTLOOPTHREADPOLL_H__
#define __EVENTLOOPTHREADPOLL_H__
#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include <functional>
class EventLoop;
class EventLoopThread;
class EventLoopThreadPoll
{
public:
    EventLoopThreadPoll(EventLoop *loop, const std::string &name);
    ~EventLoopThreadPoll();

    EventLoopThreadPoll(const EventLoopThreadPoll &rhs) = delete;
    EventLoopThreadPoll &operator=(EventLoopThreadPoll &rhs) = delete;

    void setThreadNum(int threadNum) { m_threadNum = threadNum; }
    void start(std::function<void(EventLoop *)> threadInitCallBack = std::function<void(EventLoop *)>());

    EventLoop *getNextLoop();

private:
    EventLoop *m_baseLoop;
    int m_threadNum;
    std::string m_name;

    std::vector<std::unique_ptr<EventLoopThread>> m_threads;
    std::vector<EventLoop *> m_loops;
    int m_next;
    std::atomic<bool> m_started;
};
#endif // __EVENTLOOPTHREADPOLL_H__