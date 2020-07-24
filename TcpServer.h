#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__
#include <functional>
#include <memory>
#include <utility>
#include <map>
#include <atomic>
#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "CallBacks.h"
using muduo::net::InetAddress;
class Acceptor;
class EventLoopThread;
class EventLoopThreadPoll;
class TcpServer
{
public:
    typedef std::function<void(std::shared_ptr<TcpConnection> &)> NewConnectionCallback;

    TcpServer(std::string &name, InetAddress server);
    ~TcpServer();

    // call before start
    void setThreadNum(int threadNum);

    void start();
    void setConnectionCallback(NewConnectionCallback cb) { m_connCb = std::move(cb); }
    void setMessageCacllback(MessageCallBack cb) { m_msgCb = std::move(cb); }

    TcpServer &operator=(const TcpServer &) = delete;
    TcpServer(const TcpServer &) = delete;

private:
    void handleNewConn(Socket &&socket, InetAddress &peerAddr);
    void removeConnection(TcpConnection::TcpConnectionPtr &ptr);

private:
    std::string m_name;
    std::string m_ipPort;
    std::atomic<bool> m_started;
    EventLoop *m_loop;
    std::unique_ptr<Acceptor> m_acceptor;
    NewConnectionCallback m_connCb;
    MessageCallBack m_msgCb;
    std::map<std::string, std::shared_ptr<TcpConnection>> m_connectionList;
    int m_nextConnfd;
    std::unique_ptr<EventLoopThreadPoll> m_loopThreadPoll;
};

#endif // __TCPSERVER_H__