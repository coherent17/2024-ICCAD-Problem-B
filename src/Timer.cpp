#include "Timer.h"

Timer::Timer(){}

Timer::~Timer(){}

void Timer::start(){
    start_time = std::chrono::high_resolution_clock::now();
}

void Timer::stop(){
    end_time = std::chrono::high_resolution_clock::now();
    DEBUG_TIMER("Elapsed time: " + std::to_string(std::chrono::duration<double>(end_time - start_time).count()));
}

void Timer::stop(const std::string &block){
    end_time = std::chrono::high_resolution_clock::now();
    DEBUG_TIMER("[" + block + "] " + "Elapsed time: " + std::to_string(std::chrono::duration<double>(end_time - start_time).count()));
}

double Timer::elapsed()const{
    return std::chrono::duration<double>(end_time - start_time).count();
}