#include "EPollPoller.h"
#include "Logger.h"

#include <errno.h>
#include <unistd.h>

const int kNew = -1; //channel未添加到Poller中，channel的成员index为-1
const int KAdded = 1; //channel已添加到Poller中
const int KDeleted = 2;  //channel从Poller中删除

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(KInitEventListSize) // 创建epoll_event数组
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
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
