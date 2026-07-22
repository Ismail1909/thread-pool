#ifndef  TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>

class Timer {
    using time_point = std::chrono::time_point<std::chrono::steady_clock>;

    const char* m_name;
    time_point m_start_point;

    void stop() {
        time_point m_end_point = std::chrono::steady_clock::now();
        
        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_start_point).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(m_end_point).time_since_epoch().count();
        auto duration = end - start;
        auto duration_ms = (double)duration * 0.001f;

        std::cout << m_name << " duration: " << duration << " microsecs (" << duration_ms << " ms)" << std::endl;
    }

public:
    Timer(const char* name) : m_name{name} {
        m_start_point = std::chrono::steady_clock::now();
    }

    ~Timer() {
        stop();
    }


};

#endif /* TIMER_H */