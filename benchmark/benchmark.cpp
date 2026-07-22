#include "timer.h"

#include "advanced_thread_pool.h"
#include "simple_thread_pool.h"

#include <array>

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

int main() {
    uint32_t no_of_threads = std::thread::hardware_concurrency();
    std::cout << "System's no of threads: " << no_of_threads << std::endl;

    std::cout << "## Scheduling & Synchronization Overhead ##" << std::endl;
    {
        Timer timer{"simple_thread_pool"};
        simple_thread_pool pool{no_of_threads};

        for(uint64_t i = 0 ; i < NUMBER_OF_TASKS ; ++i) {
            pool.do_work([]() {
                noop();
            });
        }
    }

    {
        Timer timer{"advanced_thread_pool"};
        advanced_thread_pool pool{no_of_threads};

        for(uint64_t i = 0 ; i < NUMBER_OF_TASKS ; ++i) {
            pool.do_work([]() {
                noop();
            });
        }
    }

    std::cout << "## Small CPU Bound Task ##" << std::endl;

    std::array<uint32_t, 8> pattern {64, 68, 72, 76, 80, 84, 88, 92};
    uint32_t pattern_size = pattern.size();
    {
        Timer timer{"simple_thread_pool"};
        simple_thread_pool pool{no_of_threads};

        for(uint64_t i = 0 ; i < NUMBER_OF_TASKS ; ++i) {
            const auto n = pattern[i % pattern_size];
            pool.do_work([n]() {
                auto result = fibonacci(n);
                do_not_optimize(result);
            });
        }
    }

    {
        Timer timer{"advanced_thread_pool"};
        advanced_thread_pool pool{no_of_threads};

        for(uint64_t i = 0 ; i < NUMBER_OF_TASKS ; ++i) {
            const auto n = pattern[i % pattern_size];
            pool.do_work([n]() {
                auto result = fibonacci(n);
                do_not_optimize(result);
            });
        }
    }

    std::cout << "## Memory Bound Task ##" << std::endl;

    {
        Timer timer{"simple_thread_pool"};
        simple_thread_pool pool{no_of_threads};

        for (uint64_t i = 0; i < NUMBER_OF_TASKS; ++i)
        {
            const uint64_t begin = i % (DATA_SIZE - CHUNK_SIZE);

            pool.do_work([&, begin]
            {
                uint64_t sum = 0;

                for (uint64_t j = begin; j < begin + CHUNK_SIZE; ++j)
                    sum += data[j];

                do_not_optimize(sum);
            });
        }
    }

    {
        Timer timer{"advanced_thread_pool"};
        advanced_thread_pool pool{no_of_threads};

        for (uint64_t i = 0; i < NUMBER_OF_TASKS; ++i)
        {
            const uint64_t begin = i % (DATA_SIZE - CHUNK_SIZE);

            pool.do_work([&, begin]
            {
                uint64_t sum = 0;

                for (uint64_t j = begin; j < begin + CHUNK_SIZE; ++j)
                    sum += data[j];

                do_not_optimize(sum);
            });
        }
    }

    std::cout << "## END BENCHMARK ##" << std::endl;
}