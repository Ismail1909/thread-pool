#include "task_queue.h"

#include <stdexcept>

task_queue::task_queue(std::size_t count) : m_work_queues{count}, m_locks{count}, m_no_of_queues{count} {
    if(count == 0) throw std::invalid_argument("Number of queue must be > 0"); 
}

auto task_queue::try_add(std::size_t index, work_item_t work_item) -> bool {
    if(index > m_no_of_queues -1) return false;

    // Reject null function
    if(!work_item) {
        throw std::invalid_argument{"Function cannot be null"};
    }

    // Create work unique pointer.
    work_item_ptr_t work_item_ptr = std::make_unique<work_item_t>(work_item);

    // lock the queue & add to it
    std::unique_lock<std::mutex> lck{m_locks[index]};
    m_work_queues[index].push(std::move(work_item_ptr));
    return true;
}

auto task_queue::try_get(std::size_t index) -> work_item_ptr_t {
    if(index > m_no_of_queues -1) return {};
    
    // If queue is busy return immediately with null
    if(!m_locks[index].try_lock()) return {};

    // If queue is empty,  unlock & return null
    if(m_work_queues[index].empty()) {
        m_locks[index].unlock();
        return {};
    }

    // Fetch the work_item then unlock and return.
    work_item_ptr_t work_item_ptr = std::move(m_work_queues[index].front());
    m_work_queues[index].pop();
    m_locks[index].unlock();
    
    return std::move(work_item_ptr);
}
