#include <thread>
#include <functional>
#include <iostream>
#include <utility>
#include "EventLoop.h"
#include "Socket.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/EventLoop.h"
#include "TcpServer.h"
#include "TcpConnection.h"
#include "muduo/net/Buffer.h"
EventLoop eventLoop;

int afercnt = 0;
int beforcnt = 0;
int g_flag = 0;
void timeThreadCb()
{
    LOG_WARN << ":!!!!!!!!!!!!!!!!!!!void timeThreadCb()";
}
void timerAfterCallBack()
{
    afercnt++;
    LOG_WARN << afercnt << "timer555CallBack:" << muduo::CurrentThread::tid();
}
void timerBeforeCallBack()
{
    beforcnt++;
    LOG_WARN
        << beforcnt
        << ":!!!!!!!!!!!!!!!!!!timer101010CallBack : " << muduo::CurrentThread::tid();
}
void testAddTimerThread(int i)
{

    // eventLoop.runEvery(i, std::bind(timeThreadCb));
}
void testAddtimer()
{
    std::vector<std::thread> v;
    for (int i = 0; i < 10; i++)
    {
        v.push_back(std::move(std::thread(testAddTimerThread, 1)));
    }
    for (int i = 0; i < 10; i++)
    {
        if (v[i].joinable())
        {
            v[i].join();
        }
    }
}
void testFuck()
{
    LOG_WARN << "test fuck";
}
void testRunInLoop()
{
    EventLoop::Functor func(std::bind(&EventLoop::quit, &eventLoop));
    eventLoop.runInLoop(std::bind(&EventLoop::queueInLoop, &eventLoop, func));
    LOG_WARN << "current thread id" << muduo::CurrentThread::tid();
    // eventLoop.runEvery(1, std::bind(timerAfterCallBack));
}

void run4()
{
    printf("run4(): pid = %d, flag = %d\n", getpid(), g_flag);
    eventLoop.quit();
}

void run3()
{
    printf("run3(): pid = %d, flag = %d\n", getpid(), g_flag);
    eventLoop.runAfter(3, run4);
    g_flag = 3;
}

void run2()
{
    printf("run2(): pid = %d, flag = %d\n", getpid(), g_flag);
    eventLoop.queueInLoop(run3);
}

void run1()
{
    g_flag = 1;
    printf("run1(): pid = %d, flag = %d\n", getpid(), g_flag);
    eventLoop.runInLoop(run2);
    g_flag = 2;
}
void testConnection(std::shared_ptr<TcpConnection> &conn)
{
    LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
}
void testMessage(TcpConnectionPtr &conn, muduo::net::Buffer *buffer, Timestamp time)
{
    std::string msg(buffer->retrieveAllAsString());
    LOG_INFO << " echo " << msg.size() << " bytes, "
             << "data received at " << time.toString();

    std::string msga(8000 * 1024, 'a');
    msga += 'b';
    if (conn->connected())
    {
        conn->send(msga);
    }
}
void testTcpServer()
{
    InetAddress svraddr("0.0.0.0", 13000);
    std::string name("echoserver");
    TcpServer server(name, svraddr);
    server.setConnectionCallback(std::bind(testConnection, std::placeholders::_1));
    server.setMessageCacllback(std::bind(testMessage, std::placeholders::_1,
                                         std::placeholders::_2,
                                         std::placeholders::_3));
    server.start();
}

int main()
{

    LOG_WARN << "main thread id" << muduo::CurrentThread::tid();
    std::thread tcpserver(testTcpServer);

    tcpserver.join();
}
