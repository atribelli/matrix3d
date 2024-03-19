// timer.h

#ifndef timer_h
#define timer_h

#include <chrono>

// Template for determining elapsed time.
// T - Type of the returned ticks.
// P - Tick period, see std::chrono::duration helper types.
// Ex:
//      timer<int, milli> millisconds;
//      ...
//      millisconds.start();
//      ...
//      auto elapsed = millisconds.elapsed();

template <typename T, typename P>
class timer {
private:
    std::chrono::steady_clock::time_point begin;

public:
    timer()  { start(); }
    ~timer() {}
    
    // Record the start or restart of a time measurement
    void start(void) {
        begin = std::chrono::steady_clock::now();
    }
    
    // Return elapsed time in units of the defined period
    T elapsed(void) {
        auto elapsed = std::chrono::steady_clock::now() - begin;
        auto ticks   = std::chrono::duration_cast<std::chrono::duration<T, P>>(elapsed);
        
        return ticks.count();
    }
};

#endif  // timer_h
