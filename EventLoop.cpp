#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>

// 防止一个线程创建多个EventLoop
__thread EventLoop *t_loopInThisThread = nullptr;

// 定义默认的Poller IO 复用的超时时间
const int kPollTimeMs = 10000;

// 创建wakeupfd,用于通知休眠的subReactor处理新来的channel
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL("eventfd error:%d \n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      callingPendingFunctors_(false),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      wakeupfd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupfd_)),
      currentActiveChannel_(nullptr)
{
    LOG_DEBUG("EventLoop created %p in thread %d \n", this, threadId_);
    if(t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n", t_loopInThisThread, threadId_);
    }
    else{
        t_loopInThisThread = this;
    }

    //设置wakeupfd的事件类型以及发生事件后的回调操作
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    //每一个eventloop都将监听wakeupChannel的EpollIN事件
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupfd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupfd_, &one, sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR("EventLoop::handleRead() reads %d bytes instead of 8\n", n);
    }
}

void EventLoop::doPendingFunctors()
{
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping \n", this);
    while(!quit_)
    {
        activeChannels_.clear();
        //监听两类fd,一种是client fd（客户端的fd）,一种是wakeupfd（用于main loop 唤醒 subloop）
        pollingReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
        for(Channel * channel : activeChannels_)
        {
            //Poller监听哪些channel发生事件了，然后上报给EventLoop，然后通知Channel处理相应的事件
            channel->handleEvent(pollingReturnTime_);
        }
        //执行当前EventLoop事件循环需要处理的回调操作
        /**
         * IO线程 main Reactor accept新用户的连接，返回一个fd，用Channel封装；这个Channel会分发给sub Reactor处理，唤醒新的loop
         * mainloop事先注册一个回调（需要subloop执行）
         * wakeup subloop后，执行下面的方法，执行之前mainloop注册的cb操作
         */
        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p stop looping.\n ", this);
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    if(!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    
}

void EventLoop::queueInLoop(Functor cb)
{
}

void EventLoop::wakeup()
{
}

void EventLoop::updateChannel(Channel *channel)
{
}

void EventLoop::removeChannel(Channel *, channel)
{
}

bool EventLoop::hasChannel(Channel *channel)
{
    return false;
}
