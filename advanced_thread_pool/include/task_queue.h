#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include <queue>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <optional>
#include <condition_variable>

using work_item_t = std::function<void(void)>;
using work_item_ptr_t = std::unique_ptr<work_item_t>;

class task_queue {

    std::queue<work_item_ptr_t>  m_work_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_stopped;

public:
    explicit task_queue();
    
    task_queue(const task_queue& other) = delete;
    task_queue(task_queue&& other) = delete;

    auto try_push(work_item_t work_item) -> bool;
    auto try_pop(work_item_ptr_t& work_item_ptr) -> bool;

    auto push(work_item_t work_item) -> void;
    auto pop(work_item_ptr_t& work_item_ptr) -> bool;

    auto done() -> void;

};

#endif // TASK_QUEUE_H