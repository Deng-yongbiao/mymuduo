#pragma once

#include "Channel.h"
#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"


#include <functional>


class EventLoop;

class InetAddress;

//运行在mainloop里面的(baseloop)
class Acceptor
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;
    Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback &cb)
    {
        newConnectionCallback_ = cb;
    }

    bool listening() const { return listening_; }

    void listen();


private:
    /**
     * 成员变量
     * 1. mainloop loop_
     * 2. 接收连接的socket
     * 
     * 4.新连接到来的回调操作，调用getnextloop函数，唤醒subloop
     * 
     */
    EventLoop *loop_; //Acceptor用的就是用户定义的那个baseloop, 也称做baseloop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listening_;

    void handleRead();
};
