#ifndef ADVANCED_THREAD_POOL_H
#define ADVANCED_THREAD_POOL_H

#include <thread>
#include <vector>
#include <atomic>

#include <functional>
#include <utility>
#include <future>
#include <type_traits>
#include <memory>

#include "task_queue.h"

// Variadic Function Params Helpers
template<typename F, typename ...Args>
using return_type = std::invoke_result_t<F, Args...>;

// for functions that require rvalue param, this make use of bind that is called when the outer bind is
// invoked, effectively casting rvalue ref on the param so it can be passed as rvalue ref
template<typename T>
auto make_bind_compatible(std::remove_reference_t<T>&& arg) {
    return std::bind(std::move<T&>, std::forward<T>(arg));
}

// Wrapping into std::ref if the provided param is lvalue reference 
// to preserve this property for std::bind
template<typename T>
std::reference_wrapper<std::remove_reference_t<T>> make_bind_compatible(std::remove_reference_t<T>& arg) {
    return std::ref(arg);
}

class advanced_thread_pool {
    using threads_t = std::vector<std::thread>;
    using task_queues_t = std::vector<task_queue>;

    task_queues_t m_task_queues;
    threads_t m_threads;
    std::size_t m_no_of_threads;
    std::atomic<std::size_t> m_current_index;
    bool m_stopped;

    auto stop() -> void;

public: 
    explicit advanced_thread_pool(std::size_t count = std::thread::hardware_concurrency());
    ~advanced_thread_pool();
    
    advanced_thread_pool(const advanced_thread_pool& other) = delete;

    advanced_thread_pool(advanced_thread_pool&& other) = delete;

    auto do_work(std::function<void(void)> work_item) -> void;

    template<typename F, typename ...Args>
    auto do_func(F&& func, Args&&... args) -> std::future<return_type<F,Args...>> {
        using task_t =  std::packaged_task<return_type<F,Args...>()>;

        task_t task(std::bind(std::forward<F>(func), make_bind_compatible<Args>(std::forward<Args>(args))...));
        auto future = task.get_future();

        // type erase packaged_task in a lambda to match the std::function<void(void))
        auto work = [task_ptr = std::make_shared<task_t>(std::move(task))]() {
            (*task_ptr)();
        };

        do_work(work);
        return future;
    }
};

#endif // ADVANCED_THREAD_POOL_H