#pragma once

/**
 * 用户使用muduo编写服务器程序
 *
 */

#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "noncopyable.h"
#include "EventLoopThreadPoll.h"
#include "Callbacks.h"

#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>
// 对外的编程暴露的接口类
class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    enum Option
    {
        kNoReusePort,
        KReusePort,
    };

    TcpServer(EventLoop *loop, 
              const InetAddress &listenAddr, 
              const std::string& nameArg, 
              Option option = kNoReusePort);

    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitcallback_ = cb; }

    void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

    // 设置subloop的个数
    void setThreadNum(int numThreads);

    // 开启服务器监听，实际上开启了mainloop的listen
    void start();

private:
    void newConnection(int sockfd, const InetAddress &peerAddr);
    void removeConnection(const TcpConnectionPtr &conn);
    void removeConnectionInLoop(const TcpConnectionPtr &conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;
    EventLoop *loop_;          // baseloop 也就是用户定义的loop
    const std::string ipPort_; // 保存服务器的ip和端口
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;              // 运行在mainloop，任务就是监听新连接事件
    std::shared_ptr<EventLoopThreadPoll> threadPool_; // one loop per thread

    ConnectionCallback connectionCallback_;       // 有新连接的回调操作
    MessageCallback messageCallback_;             // 有读写消息时的回调操作
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完成以后的回调操作

    ThreadInitCallback threadInitcallback_; // 线程初始化时的回调
    std::atomic_int started_;

    int nextConnId_;
    ConnectionMap connections_; // Tcp连接的字典
};
