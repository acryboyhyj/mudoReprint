#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__
#include <functional>
#include <memory>
#include "muduo/net/Buffer.h"
#include "muduo/base/Timestamp.h"
class TcpConnection;
using muduo::Timestamp;
using muduo::net::Buffer;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using MessageCallBack = std::function<void(Buffer *buffer, Timestamp time)>;

void defaultConnectionCallBack(TcpConnectionPtr &conn);

#endif // __CALLBACKS_H__