#pragma once
#include "noncopyable.h"


#include <functional>
#include <thread>
#include <memory>
#include <unistd.h>
#include <atomic>
#include <string>

class Thread
{
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc, const std::string &name = std::string());
    ~Thread();
    void start();
    void join();

    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string name() const { return name_; }

    static int numCreated() { return numCreated_; }
private:
    void setDefaultName();
    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_; //用shared_ptr封装的thread
    pid_t tid_;  //线程Id
    ThreadFunc func_; //thread执行的函数
    std::string name_; //线程的名字，方便于调试
    static std::atomic_int32_t numCreated_; //总共的线程数量
};
