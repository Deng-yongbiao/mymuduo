#include "EPollPoller.h"

EPollPoller::EPollPoller(EventLoop *loop) 
  : Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(KInitEventListSize)  //创建epoll_event数组
{

}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}
// channel update /remove => EventLoop updatechannel removechannel => xxx
void EPollPoller::updateChannel(Channel *channel)
{

}

void EPollPoller::removeChannel(Channel *channel)
{
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    return Timestamp();
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
}

void EPollPoller::update(int operation, Channel *channel)
{
}
