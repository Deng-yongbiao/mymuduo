#pragma once
#include "Poller.h"
#include "Timestamp.h"

#include <vector>
#include<sys/epoll.h>
/**
 * epoll的使用
 * 1. epoll_create 创建epoll对象文件描述符
 * 2. epoll_ctl   注册/修改/删除 监听的fd以及监听的事件类型    add/mod/del
 * 3. epoll_wait  阻塞返回触发监听的事件
 */
class EPollPoller : public Poller
{
public:
    EPollPoller(EventLoop *loop); // epoll_create
    ~EPollPoller() override;
    //重写基类的抽象方法
    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override; //执行epoll_wait的逻辑
    void updateChannel(Channel *channel) override; //epoll_ctl(add/mod)
    void removeChannel(Channel *channel) override; //epoll_ctl(del)
private:
    static const int KInitEventListSize = 16; //EventListd的初始长度
    //填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList * activeChannels) const;
    //更新channel通道，其实是调用epoll_ctl
    void update(int operation, Channel *channel);
    using EventList = std::vector<epoll_event>;
    int epollfd_; //epoll_create创建出来的epoll文件描述符
    EventList events_;
};
