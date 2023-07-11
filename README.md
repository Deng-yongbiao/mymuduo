## 1. Multi-Reactor 概述

Muduo库是基于Reactor模式实现的TCP网络编程库。该文章后续篇幅都是围绕Multi-reactor模型进行展开。Multi-Reactor模型如下所示（网上找的图，不是我画的）：
![tupian](img/Multi-reactor.webp)

## 2. Multi-Reactor架构的三大核心模块介绍

### 2.1 概述

Muduo库有三个核心组件支撑一个reactor实现持续的监听一组fd,并根据每个fd发生的事件调用相应的处理函数。这三个组件分别是Channel类、Poller/Epoller类以及EventLoop类。EventLoop类包含了一个Channel和Poller

### 2.2 三大组件之一：Channel类

#### 2.2.1 Channel类概述

**Channel类相当于一个文件描述符的保姆**

在IO多路复用机制中，想用监听某个文件描述符，需要把这个fd和该fd感兴趣的事件通过epoll_ctl注册到epoll对象中。当事件监听器监听到该fd发生了某个事件。事件监听器返回[发生事件的fd集合]以及[每个fd都发生了什么事件]。

Channel类则封装了一个 [fd] 和这个 [fd感兴趣事件] 以及事件监听器监听到 [该fd实际发生的事件]。同时Channel类还提供了设置该fd的感兴趣事件，以及将该fd及其感兴趣事件注册到事件监听器或从事件监听器上移除，以及保存了该fd的每种事件对应的处理函数。

#### 2.2.2 Channel类重要的成员变量

- int fd_这个Channel对象照看的文件描述符
- int events_代表fd感兴趣的事件类型集合
- int revents_代表事件监听器实际监听到该fd发生的事件类型集合，当事件监听器监听到一个fd发生了什么事件，通过Channel::set_revents()函数来设置revents值。
- EventLoop* loop这个fd属于哪个EventLoop对象，这个暂时不解释。
- read_callback_、write_callback_、close_callback_、error_callback_：这些是std::function类型，代表着这个Channel为这个文件描述符保存的各事件类型发生时的处理函数。比如这个fd发生了可读事件，需要执行可读事件处理函数，这时候Channel类都替你保管好了这些可调用函数，真是贴心啊，要用执行的时候直接管保姆要就可以了

```cpp
    EventLoop *loop_; // 事件循环
    const int fd_;    // fd, poller 监听的文件描述符
    int events_;      // 注册fd感兴趣的事件,如EPOLLIN、EPOLLOUT事件
    int revents_;     // poller选择其返回具体发生的事件
    int index_;       //

    std::weak_ptr<void> tie_;
    bool tied_;

    // channel通道能够获知fd最终发生具体的事件revents,
    //所以它负责调用具体事件的回调函数
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
```
