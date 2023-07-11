#pragma once

#include "noncopyable.h"
#include "Timestamp.h"
#include "Logger.h"

#include <functional>
#include <memory>

class EventLoop; //前置声明，只能作为指针或引用，不能调用对象的对象

/**
 * 
 * Channel理解为通道，封装了sockfd和其感兴趣的event,如EPOLLIN、EPOLLOUT事件
 * 还绑定了Poller 返回的具体事件
 */
class Channel : noncopyable
{
public:
    using EventCallback     = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);
    ~Channel();
    // fd得到Poller的通知以后，调用相应的处理事件
    void handleEvent(Timestamp receiveTime);

    // 设置回调函数
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 防止当channel 被手动remove掉，channel 还在执行回调操作、
    void tie(const std::shared_ptr<void>&);

    //返回被Poller监听的文件描述符sockfd
    int fd() const { return fd_; }
    int events() const { return events_; }

    //供Poller使用，监听到事件触发时候创建Channel调用
    void set_revents(int revt) { revents_ = revt; }

    
    //设置fd相应的事件状态
    void eableReading() { events_ |= KReadEvent; update(); }
    void disableReading() { events_ &= ~KReadEvent; update(); }
    void enableWriting() { events_ |= KWriteEvent; update(); }
    void disableWriting() { events_ &= ~KWriteEvent; update(); }
    void disableAll() { events_ = KNoneEvent; update(); }

    //返回fd当前的事件状态
    bool isNoneEvent() const { return events_ == KNoneEvent; }
    bool isWriting() const { return events_ & KWriteEvent; }
    bool isReading() const {return events_ & KReadEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    // one loop per thread
    EventLoop* ownerLoop() { return loop_; }

    void remove();
private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);
    static const int KNoneEvent;
    static const int KReadEvent;
    static const int KWriteEvent;

    EventLoop *loop_; // 事件循环
    const int fd_;    // fd, poller 监听的文件描述符
    int events_;      // 注册fd感兴趣的事件,如EPOLLIN、EPOLLOUT事件
    int revents_;     // poller选择其返回具体发生的事件
    int index_;       //

    std::weak_ptr<void> tie_;
    bool tied_;

    // channel通道能够获知fd最终发生具体的事件revents,所以它负责调用具体事件的回调函数
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};