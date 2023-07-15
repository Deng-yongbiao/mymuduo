#include "EventLoopThreadPoll.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

#include <memory>

EventLoopThreadPoll::EventLoopThreadPoll(EventLoop *baseloop, const std::string &nameArg)
    : baseLoop_(baseloop),
      name_(nameArg),
      started_(false),
      numThreads_(0),
      next_(0)
{
}

EventLoopThreadPoll::~EventLoopThreadPoll()//不需要额外的析构
{}

void EventLoopThreadPoll::start(const ThreadInitCallback &cb)
{
    started_ = true;
    
    //这个是多线程设置下
    for(int i = 0; i < numThreads_; ++i)
    {
        char buf[name_.size() + 32]; //线程的名字
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());  //底层创建线程，绑定一个新的EventLoop,并返回该loop的地址
    }
    //整个服务器只有一个线程，运行着baseloop
    if(numThreads_ == 0 && cb)
    {
        cb(baseLoop_);
    }
}

//轮询操作,从loops中取出EventLoop,获取下一个处理事件的EventLoop
EventLoop *EventLoopThreadPoll::getNextLoop()
{
    EventLoop *loop = baseLoop_; //如果是单线程，直接返回的是baseloop
    if(!loops_.empty()) //如果是多线程，轮询loops数组
    {
        loop = loops_[next_];
        ++ next_;
        if(next_ >= static_cast<int>(loops_.size()))
        {
            next_ = 0;
        }
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPoll::getAllLoops()
{
    if(loops_.empty()) //说明是单线程环境，只有一个baseloop
    {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    else
    {
        return loops_;
    }
}
