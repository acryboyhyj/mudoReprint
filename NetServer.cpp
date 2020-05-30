#include "NetServer.h"

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <functional>
#include <sys/timerfd.h>
#include "Channel.h"
#include "EventLoop.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Timestamp.h"
#include <iostream>
#include <mutex>

int g_listen;
int g_i = 0;
std::vector<int> g_vec;
std::mutex g_mutex;
int test();
NetServer::NetServer(EventLoop *eventLoop) : m_loop(eventLoop)
{
    createListen();
    m_channel = new Channel(m_loop, m_listenfd);
    m_channel->setReadCallback(
        std::bind(&NetServer::readCb, this, std::placeholders::_1));
    m_channel->enableReading();

    start();
}

void NetServer::start()
{
    // m_netsvr.reset(new std::thread(&NetServer::net, this));
    m_loop->loop();
}

void NetServer::net()
{
    while (1)
    {
        ;
    }
}

void NetServer::readCb(int fd)
{
    LOG_WARN << "netserver is read";
    // char buffer[1024] = {0};
    sockaddr_in clientAddr;
    int clntAddrlen = sizeof(sockaddr_in);
    int afd = accept(m_channel->fd(), (sockaddr *)&clientAddr,
                     (socklen_t *)&clntAddrlen);

    if (afd > 0)
    {
        printf("fd:%d\n", afd);
        // m_readsvr.reset(new std::thread(&NetServer::readThread, this, afd));
        // m_readsvr->detach();
    }
}
void NetServer::readThread(int fd)
{
    char buffer[1024] = {0};
    int retVal = read(fd, buffer, 1024);
    if (retVal <= 0)
    {

        return;
    }
    LOG_WARN << "buffer:" << buffer;
    LOG_WARN << "M_A:" << m_a;
    while (1)
    {
        memset(buffer, 0, 1024);
        retVal = read(fd, buffer, 1024);

        LOG_WARN << "buffer:" << buffer;
    }
}
NetServer::~NetServer() {}

int NetServer::createListen()
{
    struct sockaddr_in serv_addr;

    if ((m_listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("can't open stream socket");
        exit(1);
    }

    /* bind the local address, so that the cliend can send to server */
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(12000);

    if (bind(m_listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("can't bind local address");
        exit(1);
    }

    /* listen to the socket */
    listen(m_listenfd, 5);
    //
    // LOG_WARN << "listend done epollfd" << m_epollfd << " listen " <<
    // m_listenfd;
    g_listen = m_listenfd;
    return 0;
}

