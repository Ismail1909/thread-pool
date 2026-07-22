#include "timer.h"

#include "advanced_thread_pool.h"
#include "simple_thread_pool.h"

#include <array>

#define BENCHMARK(name) Timer timer##__line__{name};
#define BENCHMARK_FUNCTION() BENCHMARK(__func__)

uint32_t no_of_threads = std::thread::hardware_concurrency();
inline constexpr uint64_t NUMBER_OF_TASKS = 1'000'000;

// Prevents compiler from optimizing away the variable with no overhead.
// This works with GCC ONLY.
template<typename T>
inline void do_not_optimize(const T& value) {
    asm volatile("" : : "r,m"(value) : "memory");
}

void noop() {

}

uint64_t fibonacci(uint64_t n) {
    int a = 0 , b = 1;
    while(n--) {
        auto temp = a + b;
        a = b;
        b = temp;
    }

    return a;
}

// For Data chunks processing
constexpr uint64_t DATA_SIZE = (512*1024*1024) / sizeof(uint64_t);
std::array<uint64_t, DATA_SIZE> data{1};

constexpr uint64_t CHUNK_SIZE = (1*1024*1024) / sizeof(uint64_t); // 1 MB

constexpr uint8_t PATTERN_SIZE = 8;
std::array<uint32_t, PATTERN_SIZE> pattern {64, 68, 72, 76, 80, 84, 88, 92};

void simple_pool_noop() {
    BENCHMARK_FUNCTION();
    simple_thread_pool pool{no_of_threads};

    for(uint64_t i = 0 ; i < NUMBER_OF_TASKS ; ++i) {
        pool.do_work([]() {
            noop();
        });
    }
}

void advanced_pool_noop() {
    BENCHMARK_FUNCTION();
    advanced_thread_pool pool{no_of_threads};

    for(uint64_t i = 0 ; i < NUMBER_OF_TASKS ; ++i) {
        pool.do_work([]() {
            noop();
        });
    }
}

void noop_benchmark() {
    std::cout << "## Scheduling & Synchronization Overhead ##" << std::endl;

    simple_pool_noop();
    advanced_pool_noop();
}

void simple_pool_cpu_bound() { 
    BENCHMARK_FUNCTION();
    
    simple_thread_pool pool{no_of_threads};
    for(uint64_t i = 0 ; i < NUMBER_OF_TASKS ; ++i) {
        const auto n = pattern[i % PATTERN_SIZE];
        pool.do_work([n]() {
            auto result = fibonacci(n);
            do_not_optimize(result);
        });
    }
}

void advanced_pool_cpu_bound() {
    BENCHMARK_FUNCTION();

    advanced_thread_pool pool{no_of_threads};
    for(uint64_t i = 0 ; i < NUMBER_OF_TASKS ; ++i) {
        const auto n = pattern[i % PATTERN_SIZE];
        pool.do_work([n]() {
            auto result = fibonacci(n);
            do_not_optimize(result);
        });
    }
}

void cpu_bound_benchmark() {
    std::cout << "## Small CPU Bound Task ##" << std::endl;

    simple_pool_cpu_bound();
    advanced_pool_cpu_bound();
}

inline uint64_t array_sum(uint64_t begin, uint64_t end) {
    uint64_t sum = 0;
    for (uint64_t j = begin; j < end; ++j)
        sum += data[j];

    return sum;
}

void simple_pool_memory_bound() {
    BENCHMARK_FUNCTION();

    simple_thread_pool pool{no_of_threads};

    for (uint64_t i = 0; i < NUMBER_OF_TASKS; ++i)
    {
        const uint64_t begin = i % (DATA_SIZE - CHUNK_SIZE);

        pool.do_work([&, begin]
        {
            uint64_t sum = array_sum(begin, begin + CHUNK_SIZE);
            do_not_optimize(sum);
        });
    }
}

void advanced_pool_memory_bound() {
    BENCHMARK_FUNCTION();
    
    advanced_thread_pool pool{no_of_threads};

    for (uint64_t i = 0; i < NUMBER_OF_TASKS; ++i)
    {
        const uint64_t begin = i % (DATA_SIZE - CHUNK_SIZE);

        pool.do_work([&, begin]
        {
            uint64_t sum = array_sum(begin, begin + CHUNK_SIZE);
            do_not_optimize(sum);
        });
    }
}

void memory_bound_benchmark() {
    std::cout << "## Memory Bound Task ##" << std::endl;
    
    simple_pool_memory_bound();
    advanced_pool_memory_bound();
}

int main() {
    
    std::cout << "System's no of threads: " << no_of_threads << std::endl;

    noop_benchmark();
    
    cpu_bound_benchmark();

    memory_bound_benchmark();

    std::cout << "## END BENCHMARK ##" << std::endl;
}