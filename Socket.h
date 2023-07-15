#pragma once

#include "noncopyable.h"

#include <arpa/inet.h>

class InetAddress; // 类型前置声明

// Socket 文件描述符的包装类
class Socket : noncopyable
{
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();

    int fd() const { return sockfd_; }

    //绑定套接字地址
    void bindAddress(const InetAddress &localaddr);

    void listen();

    int accept(InetAddress *peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);

    void setReuseAddr(bool on);

    void setReusePort(bool on);

    void setKeepAlive(bool on);

private:
    const int sockfd_;
};
