#include "task_queue.h"

#include <stdexcept>

task_queue::task_queue() : m_work_queue{}, m_stopped{false} {
     
}

auto task_queue::try_push(work_item_t work_item) -> bool {
    // Reject null function
    if(!work_item) {
        throw std::invalid_argument{"Function cannot be null"};
    }

    // Don't accept new work if stopped.
    if(m_stopped) {
        return false;
    }

    // try to lock the queue & add to it
    {
        std::unique_lock<std::mutex> lck{m_mutex, std::try_to_lock};
        if(!lck) return false;

        m_work_queue.push(std::make_unique<work_item_t>(work_item));
    }
    return true;
}

auto task_queue::try_pop(work_item_ptr_t& work_item_ptr) -> bool {
    
    // If queue is busy return immediately with false
    std::unique_lock<std::mutex> lck{m_mutex, std::try_to_lock};
    
    if(!lck || m_work_queue.empty()) return false;

    // Fetch the work_item then unlock and return.
    work_item_ptr = std::move(m_work_queue.front());
    m_work_queue.pop();
    
    return true;
}

auto task_queue::push(work_item_t work_item) -> void {
    // Reject null function
    if(!work_item) {
        throw std::invalid_argument{"Function cannot be null"};
    }

    // Don't accept new work if stopped.
    if(m_stopped) {
        return;
    }

    // try to lock the queue & add to it
    {
        std::unique_lock<std::mutex> lck{m_mutex};
        m_work_queue.push(std::make_unique<work_item_t>(work_item));
    }
    m_cv.notify_one();
}

auto task_queue::pop(work_item_ptr_t &work_item_ptr) -> bool {
    {
        std::unique_lock<std::mutex> lck{m_mutex};
        m_cv.wait(lck, [this]() {
            return m_stopped || !m_work_queue.empty();
        });

        if(m_work_queue.empty()) return false;

        // Fetch the work_item then unlock and return.
        work_item_ptr = std::move(m_work_queue.front());
        m_work_queue.pop();
    }

    return true;
}

auto task_queue::done() -> void {
    m_stopped = true;
    m_cv.notify_one();
}
