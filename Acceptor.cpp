#include "Acceptor.h"
#include "Logger.h"
#include "InetAddress.h"


#include "sys/types.h"
#include "sys/socket.h"
#include <errno.h>
#include <unistd.h>

static int createNonblocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if(sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen socket create error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
: loop_(loop),
acceptSocket_(createNonblocking()),
acceptChannel_(loop, acceptSocket_.fd()),
listening_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenAddr);//绑定套接字
    //TCPserver::start()   Acceptor.listen()  有新用户的连接，执行一个回调，(connfd先打包为channel,然后唤醒subloop)
    // baseloop => acceptChannel_(listenfd) => 
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listening_ = true;
    acceptSocket_.listen(); //listen
    acceptChannel_.enableReading();
}

// listenfd 有事件发生，即有新用户连接了，会执行这个回调操作
void Acceptor::handleRead()
{
    InetAddress peerAddr;  //客户端的地址
    int connfd = acceptSocket_.accept(&peerAddr); //返回建立连接好的通讯用的套接字
    if(connfd >= 0) //连接成功，执行回调
    {
        if(newConnectionCallback_) //执行回调，这个回调其实是由TCPServer创建的
        {
            newConnectionCallback_(connfd, peerAddr); //轮询找到subloop，唤醒。分发当前新客户端的Channel
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d accept error:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        if(errno == EMFILE) //打开文件描述符太多了，到了上限
        {
            LOG_ERROR("%s:%s:%d sockfd reached limit:%d\n", __FILE__, __FUNCTION__, __LINE__, errno);
        }
    }
}
