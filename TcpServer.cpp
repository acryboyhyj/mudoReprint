#include "TcpServer.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "EventLoopThreadPoll.h"
#include <string>
TcpServer::TcpServer(std::string &name, InetAddress server) : m_name(name),
                                                              m_ipPort(server.toIpPort()),
                                                              m_started(false),
                                                              m_loop(new EventLoop()),
                                                              m_acceptor(new Acceptor(m_loop, server)),
                                                              m_nextConnfd(1),
                                                              m_loopThreadPoll(new EventLoopThreadPoll(m_loop, m_name))
{
    m_acceptor->setNewConnectionCallback(std::bind(&TcpServer::handleNewConn, this, std::placeholders::_1, std::placeholders::_2));
    m_acceptor->listen();
}

TcpServer::~TcpServer()
{
    if (m_loop)
    {
        delete m_loop;
    }
    LOG_TRACE << "TcpServer::~TcpServer [" << m_name << "] destructing";

    // for (auto &item : connections_)
    // {
    //     TcpConnectionPtr conn(item.second);
    //     item.second.reset();
    //     conn->getLoop()->runInLoop(
    //         std::bind(&TcpConnection::connectDestroyed, conn));
    // }
}

void TcpServer::setThreadNum(int threadNum)
{
    assert(threadNum >= 0);
    m_loopThreadPoll->setThreadNum(threadNum);
}
void TcpServer::start()
{

    if (!m_started.exchange(true, std::memory_order_seq_cst))
    {
        m_loopThreadPoll->start();
        m_loop->loop();
    }
}

void TcpServer::handleNewConn(Socket &&socket, InetAddress &peerAddr)
{
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", m_ipPort.c_str(), m_nextConnfd);
    ++m_nextConnfd;
    string connName = m_name + buf;

    LOG_INFO << "TcpServer::newConnection [" << m_name
             << "] - new connection [" << connName
             << "] from " << peerAddr.toIpPort();

    InetAddress localAddr(muduo::net::sockets::getLocalAddr(socket.fd()));

    EventLoop *loop = m_loopThreadPoll->getNextLoop();
    LOG_INFO << "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    std::shared_ptr<TcpConnection> tcpConnection =
        std::make_shared<TcpConnection>(loop,
                                        std::make_unique<Socket>(std::move(socket)),
                                        connName,
                                        localAddr,
                                        peerAddr);
    m_connectionList[tcpConnection->name()] = tcpConnection;
    tcpConnection->setConnectionCallBack(m_connCb);
    tcpConnection->setMessageCallBack(m_msgCb);
    tcpConnection->setCloseCallBack(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    loop->runInLoop(std::bind(&TcpConnection::connectEstablished, tcpConnection));
    LOG_INFO << "1111111111111111111111111111111111111111";
    if (m_connCb)
    {
        m_connCb(tcpConnection);
    }
    LOG_INFO << "222222222222222222222222222222222222222";
}

void TcpServer::removeConnection(TcpConnection::TcpConnectionPtr &ptr)
{
    m_loop->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << m_name
             << "] - connection " << ptr->name();

    size_t n = m_connectionList.erase(ptr->name());
    assert(n == 1);
}
