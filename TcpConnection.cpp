
#include "TcpConnection.h"
#include "EventLoop.h"
#include "Channel.h"
#include "CallBacks.h"
TcpConnection::TcpConnection(EventLoop *loop, std::unique_ptr<Socket> &&sockt, std::string &name, InetAddress &localAddr, InetAddress &peerAddr)
    : m_loop(loop),
      m_channel(new Channel(loop, sockt->fd())),
      m_socket(std::move(sockt)),
      m_name(name),
      m_localAddr(localAddr),
      m_peerAddr(peerAddr),
      m_state(kConnected),
      m_connCb(&defaultConnectionCallBack)

{
    m_channel->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    m_channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    m_channel->enableReading();
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" << m_name << "] at " << this
              << " fd=" << m_channel->fd()
              << " state=" << stateToString();
    assert(m_state == kDisconnected);
    m_state = kConnectNone;
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    m_loop->assertInLoopThread();
    int savedErrno = 0;
    ssize_t n = m_inBuffer.readFd(m_channel->fd(), &savedErrno);
    if (n > 0)
    {
        // shared_from_this()
        m_msgCallBack(&m_inBuffer, receiveTime);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleClose()
{
    setState(kDisconnected);
    //removeLoop
    m_channel->disableAll();

    //notifyuser
    if (m_connCb)
    {
        TcpConnectionPtr guardThis(shared_from_this());
        m_connCb(guardThis);
    }
    //notify netserver
    if (m_closeCb)
    {
        TcpConnectionPtr guardThis(shared_from_this());
        m_closeCb(guardThis);
    }
}

void TcpConnection::handleError()
{
    int err = muduo::net::sockets::getSocketError(m_channel->fd());
    LOG_ERROR << "TcpConnection::handleError [" << m_name
              << "] - SO_ERROR = " << err << " " << muduo::strerror_tl(err);
}
const char *TcpConnection::stateToString() const
{

    switch (m_state)
    {
    case kDisconnected:
        return "kDisconnected";
    // case kConnecting:
    //     return "kConnecting";
    case kConnected:
        return "kConnected";
    // case kDisconnecting:
    //     return "kDisconnecting";
    default:
        return "unknown state";
    }
}