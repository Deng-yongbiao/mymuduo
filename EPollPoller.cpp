#include "EPollPoller.h"
#include "Channel.h"
#include "Logger.h"

#include <errno.h>
#include <assert.h>
#include <cstring>

//channel的状态，对应channel中的index
const int kNew = -1; //表示channel还没有被处理
const int kAdded = 1; //表示channel已经添加到epoll里面去了
const int kDeleted = 2; //表示channel从epoll中删除

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(KInitEventListSize) // 创建epoll_event数组
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error:%d \n", errno);
        exit(-1);
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}
// channel update /remove => EventLoop updatechannel removechannel => xxx
/**
 *                          EventLoop
 *      ChannelList                                 Poller
 *                                           ChannelMap<fd, channel*>(注册到epoll中的channel(sockfd))
 *
 * EventLoop中ChannelList中的channel大于等于ChannelMap中的Channel
 */
void EPollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    LOG_INFO("fd=%d events=%d index=%d\n", channel->fd(), channel->events(), channel->index());
    if (index == kNew || index == kDeleted)
    {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew)
        {
            assert(channels_.find(channel) == channels_.end());
            channels_[fd] = channel;
        }
        else // index = kDeleted
        {
            assert(channels_.find(fd) != channels_.end()); // 当前channel存在于channel字典中
            assert(channels_[fd] = channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else // channel已经在poller上注册过了，所以执行DET 或者 MOD
    {
        int fd = channel->fd();
        if (channel->isNoneEvent()) // 没有感兴趣的事件，执行删除
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        { // fd上感兴趣的事件变了，执行修改
            update(EPOLL_CTL_MOD, channel);
        }
    }
}
//从Poller中删除channel
void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    channels_.erase(fd);

    int index = channel->index();
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
    
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    return Timestamp();
}
// 填写活跃的连接
void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
}

//这里真正的调用epoll库的epoll_ctl函数，执行fd的注册、修改、删除等
void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    int fd = channel->fd();
    memset(&event, 0, sizeof(event));
    event.data.ptr = channel;
    event.data.fd = fd;
    event.events = channel->events();
    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else{
            LOG_FATAL("epoll_ctl add or mod error:%d\n", errno);
        }
    }
}
