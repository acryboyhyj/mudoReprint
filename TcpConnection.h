#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__
#include "Socket.h"
#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include "Channel.h"
#include "InetAddress.h"
#include "muduo/net/Buffer.h"
#include "CallBacks.h"
#include "muduo/base/Timestamp.h"

class EventLoop;
using muduo::Timestamp;
using muduo::net::InetAddress;
using muduo::net::Socket;
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    typedef enum
    {
        kConnecting = 0,
        kConnected = 1,
        kDisConnecting = 2,
        kDisconnected = 3
    } ConnectState;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallBack = std::function<void(TcpConnectionPtr &)>;
    using CloseCallBack = std::function<void(TcpConnectionPtr &)>;
    TcpConnection(EventLoop *loop, std::unique_ptr<Socket> &&, std::string &name, InetAddress &localAddr, InetAddress &peerAddr);
    ~TcpConnection();

    void handleRead(Timestamp now);
    void setConnectionCallBack(ConnectionCallBack func) { m_connCb = std::move(func); }
    void setMessageCallBack(MessageCallBack cb) { m_msgCallBack = std::move(cb); }
    void setCloseCallBack(CloseCallBack cb) { m_closeCb = std::move(cb); };
    const InetAddress &localAddress() const { return m_localAddr; }
    const InetAddress &peerAddress() const { return m_peerAddr; }
    bool connected() const
    {
        return m_state.load(std::memory_order_seq_cst) == kConnected;
    }
    bool disconnected() const
    {
        return m_state.load(std::memory_order_seq_cst) == kDisconnected;
    }
    std::string name()
    {
        return m_name;
    }

    bool connectioned();
    void setState(ConnectState state)
    {
        m_state.store(state, std::memory_order_seq_cst);
    };

    //write
    void send(std::string &msg);

    void shutDown();

    void connectEstablished();

private:
    void shutDownInLoop();
    void sendInLoop(std::string &msg);

private:
    void handleClose();
    void handleError();
    void handleWrite();
    void send();
    const char *stateToString() const;

private:
    EventLoop *m_loop;
    std::unique_ptr<Channel> m_channel;
    std::unique_ptr<Socket> m_socket;
    std::string m_name;
    InetAddress m_localAddr;
    InetAddress m_peerAddr;
    std::atomic<ConnectState> m_state;
    MessageCallBack m_msgCallBack;
    ConnectionCallBack m_connCb;
    CloseCallBack m_closeCb;

    Buffer m_inBuffer;
    Buffer m_outBuffer;
};
#endif // __TCPCONNECTION_H__