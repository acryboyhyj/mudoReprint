#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__
#include <functional>
#include <memory>
#include "Socket.h"
#include "SocketsOps.h"
#include "Channel.h"
#include "InetAddress.h"
#include "EventLoop.h"
using muduo::net::InetAddress;
using muduo::net::Socket;
namespace sock = muduo::net::sockets;
class Acceptor
{
public:
    typedef std::function<void(Socket &&, InetAddress &)> NewConnectionCallback;
    Acceptor(EventLoop *loop, InetAddress &serverAddr) : m_loop(loop),
                                                         m_serverAddr(serverAddr),
                                                         m_listenSocket(sock::createNonblockingOrDie(serverAddr.family())),
                                                         m_channel(new Channel(m_loop, m_listenSocket.fd()))
    {
        LOG_WARN << serverAddr.toIpPort();
        m_listenSocket.setReuseAddr(true);
        m_listenSocket.bindAddress(serverAddr);

        m_channel->setReadCallback(std::bind(&Acceptor::handleRead, this));
    }

    ~Acceptor()
    {
        m_channel->disableAll();
        m_channel->remove();
    }
    void setNewConnectionCallback(NewConnectionCallback cb) { m_acceptReadCb = std::move(cb); }
    void handleRead();

    void listen()
    {
        m_loop->assertInLoopThread();
        m_listenSocket.listen();
        m_channel->enableReading();
    }

private:
    EventLoop *m_loop;
    InetAddress m_serverAddr;
    Socket m_listenSocket;
    NewConnectionCallback m_acceptReadCb;
    std::unique_ptr<Channel> m_channel;
};
#endif // __ACCEPTOR_H__