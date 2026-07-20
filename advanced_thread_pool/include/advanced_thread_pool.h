#ifndef ADVANCED_THREAD_POOL_H
#define ADVANCED_THREAD_POOL_H

#include <thread>
#include <vector>

#include "task_queue.h"

class advanced_thread_pool {
    using threads_t = std::vector<std::thread>;
    using task_queues_t = std::vector<task_queue>;

    task_queues_t m_task_queues;
    threads_t m_threads;
    std::size_t m_no_of_threads;
    std::size_t m_current_index;
    bool m_stopped;

    auto stop() -> void;

public: 
    explicit advanced_thread_pool(std::size_t count = std::thread::hardware_concurrency());
    ~advanced_thread_pool();
    
    advanced_thread_pool(const advanced_thread_pool& other) = delete;

    advanced_thread_pool(advanced_thread_pool&& other) = delete;

    auto do_work(work_item_t work_item) -> void;
};

#endif // ADVANCED_THREAD_POOL_H