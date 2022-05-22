#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

// 异步写日志的日志队列

template <typename T>
class LockQueue
{
public:
    LockQueue() {}
    ~LockQueue() {}

public:
    // 多个 worker 线程都会写日志 queue,
    void Push(const T &data)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_queue.push(data);
        m_cond.notify_one();
    }

    // 一个线程读日志 queue, 写日志文件,
    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        while (m_queue.empty())
        {
            // 日志队列为空, 线程进入 wait 状态,
            m_cond.wait(lock);
        }

        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mtx;
    std::condition_variable m_cond;
};
