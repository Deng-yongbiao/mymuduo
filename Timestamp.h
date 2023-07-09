#pragma once
#include <iostream>
#include <string>

class Timestamp
{
public:
    Timestamp();
    //防止隐式转换
    explicit Timestamp(int64_t microSecondSinceEpoch);
    //获取当前的时间
    static Timestamp now();
    std::string toString() const;
private:
    //用int64代表时间
    int64_t microSecondSinceEpoch_; 
};