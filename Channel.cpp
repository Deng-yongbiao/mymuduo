#include "Channel.h"

#include <sys/epoll.h>

const int Channel::KNoneEvent = 0;
const int Channel::KReadEvent = EPOLLIN | EPOLLPRI; //连接到达；有数据来临；有紧急的数据可读，有带外数据到来
const int Channel::KWriteEvent = EPOLLOUT; //有数据要写

Channel::Channel(EventLoop *loop, int fd) : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false)
{
}

Channel::~Channel()
{
}

void Channel::handleEvent(Timestamp receiveTime)
{
    if (tied_)
    {
        std::shared_ptr<void> guard = tie_.lock(); // 从weak提升到shared
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}

// 在Channel所属的EventLoop中，把当前的Channel删除掉，其实就是从ChannelList中删除
void Channel::remove()
{
    loop_->removeChannel(this);
}

/**
 * 当改变Channel所表示fd的event事件后，update负责在Poller中更改相应的事件:epoll_ctl
 * EventLoop => ChannelList   Poller
 */
void Channel::update()
{
    // 通过Channel所属的EventLoop，调用Poller相应的方法，注册fd的event事件
    // TODO
    loop_->updateChannel(this);
}

/**
 * @brief 根据poller通知的channel发生的具体事件，由channel调用具体的回调函数
 * 
 * @param receiveTime 接受时间戳
 */
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d", revents_);
    //如果触发关闭事件，调用关闭的回调函数
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) 
    {
        if(closeCallback_) {
            closeCallback_();
        }
    }
    //如果触发错误事件，调用错误的回调函数
    if(revents_ & EPOLLERR)
    {
        if(errorCallback_) {
            errorCallback_();
        }
    }
    //如果触发可读事件，调用可读的回调函数
    if(revents_ & (EPOLLIN | EPOLLPRI))
    {
        if(readCallback_) {
            readCallback_(receiveTime);
        }
    }
    //如果触发发送事件，调用发送的回调函数
    if(revents_ & EPOLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }
}
