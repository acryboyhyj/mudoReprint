
#include "CallBacks.h"
#include "muduo/base/Logging.h"
#include "InetAddress.h"
#include "TcpConnection.h"
void defaultConnectionCallBack(TcpConnectionPtr &conn)
{

    LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
}
