#include "EPollPoller.h"
#include "Logger.h"

#include <errno.h>
#include <unistd.h>

const int kNew = -1;
const int KAdded = 1;
const int KDeleted = 2;

EPollPoller::EPollPoller(EventLoop *loop) 
  : Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(KInitEventListSize)  //创建epoll_event数组
{
    if(epollfd_ < 0) {
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
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
