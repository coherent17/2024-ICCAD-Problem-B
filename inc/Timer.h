#ifndef _TIMER_H_
#define _TIMER_H_

#include <chrono>
#include <iostream>

#ifdef ENABLE_DEBUG_TIMER
#define DEBUG_TIMER(message) std::cout << "[TIMER] " << message << std::endl
#else
#define DEBUG_TIMER(message)
#endif

class Timer{
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;

public:
    Timer();
    ~Timer();
    void start();
    void stop();
    void stop(const std::string &block);
    double elapsed()const;
};

#endif