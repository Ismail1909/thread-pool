#ifndef SIMPLE_THREAD_POOL_H // SIMPLE_THREAD_POOL_H
#define SIMPLE_THREAD_POOL_H

#include <queue>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable> 

class simple_thread_pool {
    using work_item_t = std::function<void(void)>;
    using work_item_ptr_t = std::unique_ptr<work_item_t>;
    using threads_t = std::vector<std::thread>;
    
    std::mutex m_lock_queue;
    std::condition_variable m_cv;
    std::queue<work_item_ptr_t> m_work_queue;
    threads_t m_threads;


public: 
    explicit simple_thread_pool(std::size_t count);
    ~simple_thread_pool();
    
    simple_thread_pool(const simple_thread_pool& other) = delete;

    simple_thread_pool(simple_thread_pool&& other) = delete;

    void do_work(work_item_t work_item);
};

#endif // SIMPLE_THREAD_POOL_H
