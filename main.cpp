#include "simple_thread_pool.h"

#include<iostream>
#include <mutex>

std::mutex cout_work_guard;

int main() {
    std::cout << "Program Start" << std::endl;

    simple_thread_pool threadPool{8};

    for(std::uint32_t i = 0 ; i < 50 ; ++i) {
        threadPool.do_work([i_copy = i]() {
            std::unique_lock lck{cout_work_guard};
            std::cout<< "doing work " << i_copy << " ...." << std::endl;
        });
    }
}
