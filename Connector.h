#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__
#include <string>
#include <memory>
#include "InetAddress.h"
#include "Socket.h"

class Channel;
class EventLoop;
using muduo::net::InetAddress;
using muduo::net::Socket;
class Connector
{
public:
    Connector(EventLoop *loop, InetAddress serverAddr, std::string &name);
    ~Connector();
    int connect();
    void connecting(int sockfd);
    void retry(int sockfd);

private:
    void handleWrite();

private:
    EventLoop *m_loop;
    InetAddress m_serverAddr;
    std::string m_name;
    std::unique_ptr<Channel> m_channel;
};
#endif // __CONNECTOR_H__