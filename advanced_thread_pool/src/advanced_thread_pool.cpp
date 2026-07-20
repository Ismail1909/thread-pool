#include "advanced_thread_pool.h"

#include <stdexcept>
#include <mutex>
#include <iostream>

std::mutex cout_guard;

inline auto advanced_thread_pool::stop() -> void {
    m_stopped = true;
    for(auto& task_queue : m_task_queues) {
        task_queue.done();
    }

    for(auto&  thread : m_threads ) {
        if(thread.joinable()) {
            thread.join();
        }
    }
}

advanced_thread_pool::advanced_thread_pool(std::size_t count) :
m_task_queues{count}, m_no_of_threads{count}, m_stopped{false}, m_current_index{0} {
    if(count == 0) throw std::invalid_argument("Number of threads must be > 0"); 

    // Init threads & task queues
    m_threads.reserve(count);

    try {
        for(std::size_t i = 0 ; i < m_no_of_threads ; ++i) {
            m_threads.emplace_back([this, index = i](){
                while(true) {
                    work_item_ptr_t work_ptr;
                    for(std::size_t i = 0 ; i < m_no_of_threads ; ++i) {
                        if(m_task_queues[(index + i) % m_no_of_threads].try_pop(work_ptr)) break;
                    }

                    if(!work_ptr && !m_task_queues[index].pop(work_ptr)) break;

                    try {
                            (*work_ptr)();
                        }
                        catch(const std::exception& ex) {
                            std::unique_lock cerr_lock{cout_guard};
                            std::cerr << "Task threw exception: " << ex.what() << std::endl;
                        }
                        catch(...) {
                            std::unique_lock cerr_lock{cout_guard};
                            std::cerr << "Task threw unknown exception" << std::endl;
                        }
                }
            });
        };
    } 
    catch(const std::system_error& ex) {
        stop();       
        throw;
    }
}

advanced_thread_pool::~advanced_thread_pool() {
    stop();
}

auto advanced_thread_pool::do_work(work_item_t work_item) -> void {
    auto index = (m_current_index++) % m_no_of_threads;

    for(std::size_t i = 0 ; i < m_no_of_threads ; ++i) {
        if(m_task_queues[(index + i) % m_no_of_threads].try_push(work_item)) return;
    }
    m_task_queues[index].push(work_item);
}
