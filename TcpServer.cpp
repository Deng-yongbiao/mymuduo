#include "TcpServer.h"
#include "Logger.h"

#include <functional>

EventLoop *checkloopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainloop is null!\n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg, Option option)
    : loop_(checkloopNotNull(loop)),
      ipPort_(listenAddr.toIpPort()),
      name_(nameArg),
      acceptor_(new Acceptor(loop_, listenAddr, option == KReusePort)),
      threadPool_(new EventLoopThreadPoll(loop_, name_)),
      connectionCallback_(),
      messageCallback_(),
      started_(0),
      nextConnId_(1)
{
    //当有新用户连接时，会执行TCPServer::newConnection
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, 
                                        this, 
                                        std::placeholders::_1,
                                        std::placeholders::_2));
    
}
// 设置subloop的个数
void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}
// 开启服务器监听，loop.loop()方法
void TcpServer::start()
{
    if(started_ ++ == 0)  //防止一个TCPServer被start多次
    {
        threadPool_->start(threadInitcallback_); //启动底层loop的线程池
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{
    
}
