#include "EPollPoller.h"

EPollPoller::EPollPoller(EventLoop *loop) 
  : Poller(loop),
    epollfd_(::epoll_create1)
{

}

EPollPoller::~EPollPoller()
{
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    return Timestamp();
}

void EPollPoller::updateChannel(Channel *channel)
{
}

void EPollPoller::removeChannel(Channel *channel)
{
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
}

void EPollPoller::update(int operation, Channel *channel)
{
}
