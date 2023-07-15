#pragma once
#include "noncopyable.h"

#include <functional>
#include <string>
#include <vector>
#include <memory>

class EventLoop;
class EventLoopThread;



class EventLoopThreadPoll : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    EventLoopThreadPoll(EventLoop* baseloop, const std::string& nameArg);
    ~EventLoopThreadPoll();

    void setThreadNum(int numThreads){ numThreads_ = numThreads; }

    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    //用于多线程设置下，baseloop(mainloop)会默认以轮询的方式分配channel给subloop
    EventLoop* getNextLoop();

    std::vector<EventLoop*> getAllLoops();

    bool started() const { return started_; }

    const std::string name() { return name_; }

private:
    //这个可以认为是基本的loop，我们创建服务器的时候，需要创建一个baseloop，就在这里定义
    //如果我们不手动的设置线程数量(单线程)，那么这个baseloop就是同时负责IO事件和工作事件，也就是只有一个mainloop
    EventLoop *baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};
