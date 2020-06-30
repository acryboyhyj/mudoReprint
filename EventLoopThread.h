#ifndef __EVENTLOOPTHREAD_H__
#define __EVENTLOOPTHREAD_H__
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "EventLoop.h"
#include <functional>
class EventLoopThread
{
public:
    EventLoopThread(char *buf, std::function<void(EventLoop *)> threadInitCb);
    ~EventLoopThread();

    EventLoop *startLoop();

private:
    void threadFunc();

private:
    std::string m_name;
    std::unique_ptr<std::thread> m_threadptr;
    std::mutex m_mutex;
    std::condition_variable m_loopInitedCond;
    EventLoop *m_loop;
    std::function<void(EventLoop *)> m_threadInitCb;
};
#endif // __EVENTLOOPTHREAD_H__