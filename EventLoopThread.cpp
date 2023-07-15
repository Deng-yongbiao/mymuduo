#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
    : loop_(nullptr),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::threadFunc, this), name),
      mutex_(),
      cond_(),
      callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop()
{
    thread_.start(); //执行的是EventLoopThread::threadFunc函数
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr)
        {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

//下面这个方法是在单独的新线程里面运行的，由thread start方法启动
void EventLoopThread::threadFunc()
{
    EventLoop loop; //创建一个独立的eventloop,和上面的线程是一一对应的，这里就nullptr是所谓的one loop per thread模型
    if(callback_)
    {
        callback_(&loop);
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop(); //EventLoop loop ==> Poller.poll()

    //如果能到下面，说明服务器要关闭了
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}
