#include "advanced_thread_pool.h"

#include<iostream>
#include <mutex>

std::mutex cout_work_guard;

int add(int a, int b) {
    return a + b;
}

std::vector<std::future<int>> results;

int main() {
    std::cout << "Program Start" << std::endl;

    advanced_thread_pool threadPool{8};

    for(std::uint32_t i = 0 ; i < 10 ; ++i) {
        results.emplace_back(threadPool.do_func(add,1,2));
    }

    int i = 1;
    for(auto& r : results) {
        std::cout << "Result " << i++ << ": " << r.get() << std::endl;
    }
}
