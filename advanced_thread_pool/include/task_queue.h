#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <optional>

using work_item_t = std::function<void(void)>;
using work_item_ptr_t = std::unique_ptr<work_item_t>;

class task_queue {
    std::size_t m_no_of_queues;
    std::vector<std::mutex> m_locks;
    std::vector<std::queue<work_item_ptr_t>>  m_work_queues;

public:
    explicit task_queue(std::size_t count);
    
    task_queue(const task_queue& other) = delete;
    task_queue(task_queue&& other) = delete;

    auto try_add(std::size_t index, work_item_t work_item) -> bool;
    auto try_get(std::size_t index) -> work_item_ptr_t;

};

#endif // TASK_QUEUE_H