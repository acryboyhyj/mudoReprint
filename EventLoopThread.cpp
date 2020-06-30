#include "EventLoopThread.h"
EventLoopThread::EventLoopThread(char *buf, std::function<void(EventLoop *)> threadInitCb) : m_name(buf), m_threadptr(nullptr), m_loop(nullptr)
{
}

EventLoopThread::~EventLoopThread()
{
    if (m_threadptr != nullptr)
    {
        if (m_threadptr->joinable())
        {
            m_loop->quit();
            m_threadptr->join();
        }
    }
}

EventLoop *EventLoopThread::startLoop()
{
    {
        m_threadptr.reset(new std::thread(&EventLoopThread::threadFunc, this));
        std::unique_lock<std::mutex>
            lock(m_mutex);
        m_loopInitedCond.wait(lock, [this]() {
            return this->m_loop != nullptr;
        });
    }
    return m_loop;
}

void EventLoopThread::threadFunc()
{

    EventLoop loop;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_loop = &loop;
        m_loopInitedCond.notify_one();
    }
    m_loop->loop();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_loop = NULL;
}