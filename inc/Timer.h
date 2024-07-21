#ifndef _TIMER_H_
#define _TIMER_H_

#include <chrono>
#include <iostream>

class Timer{
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;

public:
    Timer();
    ~Timer();
    void start();
    void stop();
    double elapsed()const;
};

#endif