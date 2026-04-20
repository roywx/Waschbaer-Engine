// Basic RAII Timer class 
#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>
#include <string>

class Timer {
public:
    explicit Timer(std::string name) 
        : _timerName(name), _startTime(std::chrono::high_resolution_clock::now()) {}

    ~Timer(){
        auto endTime = std::chrono::high_resolution_clock::now();    
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - _startTime).count();
        std::cout << "[Timer] " << _timerName << ": " <<  duration << "ms\n";
    };

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;

private:
    std::string _timerName; 
    std::chrono::time_point<std::chrono::high_resolution_clock> _startTime;
};

#define ENABLE_TIMERS 1

#if ENABLE_TIMERS
    #define SCOPED_TIMER_CAT2(a,b) a##b
    #define SCOPED_TIMER_CAT(a,b)  SCOPED_TIMER_CAT2(a,b)
    #define SCOPED_TIMER(name) Timer SCOPED_TIMER_CAT(_scoped_timer_, __LINE__)(name)
#else
    #define SCOPED_TIMER(name) ((void)0)
#endif

#endif