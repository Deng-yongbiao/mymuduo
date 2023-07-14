#include "EPollPoller.h"
#include "Channel.h"
#include "Logger.h"

#include <errno.h>
#include <assert.h>
#include <cstring>
#include <unistd.h>

// channel的状态，对应channel中的index
const int kNew = -1;    // 表示channel还没有被处理
const int kAdded = 1;   // 表示channel已经添加到epoll里面去了
const int kDeleted = 2; // 表示channel从epoll中删除

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
    LOG_INFO("func=%s fd=%d events=%d index=%d\n", __FUNCTION__, channel->fd(), channel->events(), channel->index());
    if (index == kNew || index == kDeleted)
    {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else // index = kDeleted   // 当前channel存在于channel字典中，但是没有加入epoll中
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] = channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else // channel已经在channelMap上注册过了，所以执行DEL 或者 MOD
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
// 从Poller中删除channel,从Map和epoll中同时删除
void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    channels_.erase(fd);
    LOG_INFO("func=%s fd=%d events=%d index=%d\n", __FUNCTION__, channel->fd(), channel->events(), channel->index());
    int index = channel->index();
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

// 执行的是epoll_wait函数,EventLopp实际上调用这个函数
// activeChannels是一个channel数组，是一个指针类型，一开始可能是空的，在这里传回EventLoop
Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    // 实际上应该用LOG_DEBUG更为合理
    LOG_INFO("func=%s =>fd totoal count:%lu\n", __FUNCTION__, channels_.size());
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int savErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0)
    {
        LOG_INFO("%d events happened", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (numEvents == static_cast<int>(events_.size())) // 需要扩容
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_DEBUG("%s timeout! \n", __FUNCTION__);
    }
    else
    {
        if (savErrno != EINTR)
        {
            errno = savErrno;
            LOG_ERROR("EPollerPoller::poll() error!\n");
        }
    }
    return now;
}
// 填写活跃的连接
void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for(int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);   //EventLoop就拿到了它的Poller给他返回的所有发生事件channel的列表了
    }
}

// 这里真正的调用epoll库的epoll_ctl函数，执行fd的注册、修改、删除等
void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    int fd = channel->fd();
    memset(&event, 0, sizeof(event));
    event.data.ptr = channel;
    event.data.fd = fd;
    event.events = channel->events();
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add or mod error:%d\n", errno);
        }
    }
}
