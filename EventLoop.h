#pragma once
#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

class Channel;
class Poller;

//事件循环类   主要包含了两个大模块，  一个是Channel    一个Poller (epoll的抽象，可能还有poll)
class EventLoop
{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

    //开启事件循环
    void loop(); 
    //退出事件循环
    void quit();

    Timestamp pollReturnTime() const { return pollingReturnTime_; }

    //在当前Loop中循环
    void runInLoop(Functor cb);
    //把cb放入队列中，唤醒loop所在的线程执行cb
    void queueInLoop(Functor cb);
    //用来唤醒Loop所在的线程
    void wakeup();

    //由channel调用，然后这个函数调用Poller里面的方法
    void updateChannel(Channel* channel);
    void removeChannel(Channel*, channel);
    bool hasChannel(Channel* channel);
    //判断EventLoop对象是否在自己的线程里面
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
private:
    void handleRead(); //wake up
    void doPendingFunctors(); //执行回调

    using ChannelList = std::vector<Channel*>;

    std::atomic<bool> looping_;  //原子操作，通过CAS实现
    std::atomic<bool> quit_;  //标志退出loop循环
    

    const pid_t threadId_;  //记录当前Loop当前线程的id，一个线程一个Loop,一个Loop多个Channel


    Timestamp pollingReturnTime_; //poller返回发生事件channels的时间
    std::unique_ptr<Poller> poller_;

    //调用eventwakeup
    int wakeupfd_; //当mainLoop获取一个新用户的channel，通过轮询算法选择一个subloop，通过该成员唤醒subloop处理channel
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_; //所有发生事件的channel

    std::atomic<bool> callingPendingFunctors_; //标识当前Loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_; //存储Loop需要执行的回调操作
    std::mutex mutex_; //互斥锁用于保护上面的vector容器的线程安全操作
};
