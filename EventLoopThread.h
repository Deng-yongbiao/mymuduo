#pragma once
#include "noncopyable.h"
#include "Thread.h"

#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>

class EventLoop;

//这个类的作用其实是把一个EventLoop和一个线程进行绑定，实现真正的one loop per thread
//执行startLoop方法，首先会调用新开启一个线程，这个线程执行EventLoopThread::threadFunc方法，
//这个方法会创建出一个EventLoop实例，并且执行初始化回调函数ThreadInitCallback,然后执行loop方法循环，调用Poller的poll进行阻塞监听
class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                    const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop(); //开启循环


private:
    void threadFunc();

    EventLoop *loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;  //这个callback是新开启线程执行的回调函数
};

