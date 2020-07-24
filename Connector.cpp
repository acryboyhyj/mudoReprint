
#include "Connector.h"
#include "Channel.h"
#include "EventLoop.h"
#include <functional>
#include "muduo/base/Logging.h"
Connector::Connector(EventLoop *loop, InetAddress serverAddr, std::string &name) : m_serverAddr(serverAddr), m_name(name)
{
}

Connector::~Connector()
{
}

int Connector::connect()
{
    int sockfd = muduo::net::sockets::createNonblockingOrDie(AF_INET);
    int ret = muduo::net::sockets::connect(sockfd, m_serverAddr.getSockAddr());
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting(sockfd);
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
        muduo::net::sockets::close(sockfd);
        break;

    default:
        LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
        muduo::net::sockets::close(sockfd);
        // connectErrorCallback_();
        break;
    }
    // return conn;
}

void Connector::connecting(int sockfd)
{
    LOG_WARN << "connecting";
    m_channel.reset(new Channel(m_loop, sockfd));
    m_channel->setWriteCallback(std::bind(&Connector::handleWrite, this));
    m_channel->enableWrite();
    Channel *chnel = m_channel.get();
    m_loop->updateChannel(chnel);
}

void Connector::retry(int sockfd)
{
    ;
}
void Connector::handleWrite()
{

    m_channel->remove();
    m_channel.reset(nullptr);
}