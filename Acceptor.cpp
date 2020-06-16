#include "Acceptor.h"
void Acceptor::handleRead()
{
    LOG_WARN << "acceptor handle read";
    m_loop->assertInLoopThread();
    InetAddress peer;
    int connfd = m_listenSocket.accept(&peer);
    if (connfd >= 0)
    {
        // string hostport = peerAddr.toIpPort();
        // LOG_TRACE << "Accepts of " << hostport;

        if (m_acceptReadCb)
        {

            m_acceptReadCb(Socket(connfd), peer);
        }

        else
        {
            LOG_TRACE << "m_acceptReadCb  nulll ";
            sock::close(connfd);
        }
    }
    else
    {
        LOG_SYSERR << "in Acceptor::handleRead";
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of libev.
        if (errno == EMFILE)
        {
            // ::close(idleFd_);
            // idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
            // ::close(idleFd_);
            //idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
