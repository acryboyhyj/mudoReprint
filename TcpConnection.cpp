
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
      m_state(kConnecting),
      m_connCb(&defaultConnectionCallBack)

{
    m_channel->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    m_channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    m_channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" << m_name << "] at " << this
              << " fd=" << m_channel->fd()
              << " state=" << stateToString();
    assert(m_state == kDisconnected);
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    m_loop->assertInLoopThread();
    int savedErrno = 0;
    ssize_t n = m_inBuffer.readFd(m_channel->fd(), &savedErrno);
    if (n > 0)
    {
        auto guradThis = shared_from_this();
        m_msgCallBack(guradThis, &m_inBuffer, receiveTime);
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

void TcpConnection::send(std::string &msg)
{
    if (m_state == kConnected)
    {
        m_loop->runInLoop((std::bind(&TcpConnection::sendInLoop,
                                     this,
                                     msg)));
    }
}
void TcpConnection::sendInLoop(std::string &msg)
{
    m_loop->assertInLoopThread();
    // nomal send
    //send half need to save to buffer
    // if outBuffer have write need wait;
    int len = msg.size();
    int remain = len;
    bool faultErroed = false;
    ssize_t n = 0;
    if (m_state == kDisconnected)
    {
        LOG_WARN << "disconnected, give up writing";
        return;
    }
    if (!m_channel->isWriting() && m_outBuffer.readableBytes() == 0)
    {

        n = muduo::net::sockets::write(m_socket->fd(), msg.c_str(), msg.size());
        LOG_ERROR << "1111111111111111::::::" << n;
        if (n < 0)
        {
            n = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_SYSERR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                {
                    faultErroed = true;
                }
            }
        }
        else
        {
            remain = len - n;
            // if (remain == 0 && writeCompleteCallback_)
            // {
            //     m_loop->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            // }
        }
    }
    assert(remain <= len);
    if (!faultErroed && remain > 0)
    {
        size_t oldLen = m_outBuffer.readableBytes();
        // if (oldLen + remain >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
        // {
        //     m_loop->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remain));
        // }
        // m_outBuffer.append(static_cast<const char *>(data) + n, remain);
        m_outBuffer.append(msg.substr(n - 1, remain));
        if (!m_channel->isWriting())
        {
            m_channel->enableWrite();
        }
    }
}
void TcpConnection::shutDown()
{
    // FIXME: use compare and swap
    if (m_state == kConnected)
    {
        setState(kDisConnecting);
        // FIXME: shared_from_this()?
        m_loop->runInLoop(std::bind(&TcpConnection::shutDownInLoop, this));
    }
}

void TcpConnection::shutDownInLoop()
{
    m_loop->assertInLoopThread();
    if (!m_channel->isWriting())
    {
        // we are not writing
        m_socket->shutdownWrite();
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

void TcpConnection::handleWrite()
{
    m_loop->assertInLoopThread();
    if (m_channel->isWriting())
    {
        ssize_t n = muduo::net::sockets::write(m_socket->fd(), m_outBuffer.retrieveAllAsString().c_str(), m_outBuffer.readableBytes());

        if (n <= 0)
        {
            // shutDownInloop();
            muduo::net::sockets::shutdownWrite(m_socket->fd());
        }
        else
        {
            m_outBuffer.retrieve(n);
            if (m_outBuffer.readableBytes() == 0)
            {
                m_channel->disableWrite();
                // if (writeCompleteCallback_)
                // {
                //     m_loop->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                // }
                if (m_state == kDisConnecting)
                {
                    shutDownInLoop();
                }
            }
        }
    }
}

const char *TcpConnection::stateToString() const
{

    switch (m_state)
    {
    case kDisconnected:
        return "kDisconnected";
    case kConnecting:
        return "kConnecting";
    case kConnected:
        return "kConnected";
    case kDisConnecting:
        return "kDisConnecting";
    default:
        return "unknown state";
    }
}
void TcpConnection::connectEstablished()
{
    m_loop->assertInLoopThread();

    assert(m_state == kConnecting);
    setState(kConnected);
    // channel_->tie(shared_from_this()); //why
    m_channel->enableReading();
    std::shared_ptr<TcpConnection> guradThis = shared_from_this();
    m_connCb(guradThis);
}