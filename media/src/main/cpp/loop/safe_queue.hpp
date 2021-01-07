//
// Created by scliang on 1/5/21.
//

#ifndef STARS_SAFE_QUEUE_HPP
#define STARS_SAFE_QUEUE_HPP

#include <mutex>
#include <memory>
#include <condition_variable>
#include <queue>

namespace media {

template<class T, class Container = std::queue<T>>
class safe_queue {
public:
    safe_queue() = default;
    ~safe_queue() = default;

public:
    template<class Element>
    void push(Element &&element) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(std::forward<Element>(element));
        cond.notify_one();
    }

    void wait_and_pop(T &t) {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [this]() {
            return !queue.empty();
        });

        t = std::move(queue.front());
        queue.pop();
    }

    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [this]() {
            return !queue.empty();
        });

        std::shared_ptr<T> t_ptr = std::make_shared<T>(queue.front());
        queue.pop();
        return t_ptr;
    }

    bool try_pop(T &t) {
        std::lock_guard<std::mutex> lock(mutex);
        if (queue.empty()) {
            return false;
        }

        t = std::move(queue.front());
        queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lock(mutex);
        if (queue.empty()) {
            return std::shared_ptr<T>();
        }

        std::shared_ptr<T> t_ptr = std::make_shared<T>(queue.front());
        queue.pop();
        return t_ptr;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }

private:
    safe_queue(safe_queue&&) = delete;
    safe_queue(const safe_queue&) = delete;
    safe_queue& operator=(safe_queue&&) = delete;
    safe_queue& operator=(const safe_queue&) = delete;

private:
    mutable std::mutex mutex;
    std::condition_variable cond;
    Container queue;
};

} //namespace media

#endif //STARS_SAFE_QUEUE_HPP
