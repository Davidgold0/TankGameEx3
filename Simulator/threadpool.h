#pragma once
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(std::size_t workers)
        : stop_(false), active_(0) {
        for (std::size_t i = 0; i < workers; ++i) {
            threads_.emplace_back([this] {
                for (;;) {
                    std::function<void()> job;
                    {
                        std::unique_lock<std::mutex> lk(m_);
                        cv_jobs_.wait(lk, [this] { return stop_ || !jobs_.empty(); });
                        if (stop_ && jobs_.empty()) return;
                        job = std::move(jobs_.front());
                        jobs_.pop();
                        ++active_;
                    }
                    try { job(); }
                    catch (...) { /* swallow or log elsewhere */ }
                    {
                        std::lock_guard<std::mutex> lk(m_);
                        --active_;
                        if (jobs_.empty() && active_ == 0) cv_idle_.notify_all();
                    }
                }
            });
        }
    }

    ~ThreadPool() { shutdown(); }

    template<class F>
    void submit(F&& f) {
        {
            std::lock_guard<std::mutex> lk(m_);
            if (stop_) return;
            jobs_.emplace(std::forward<F>(f));
        }
        cv_jobs_.notify_one();
    }

    void wait_idle() {
        std::unique_lock<std::mutex> lk(m_);
        cv_idle_.wait(lk, [this] { return jobs_.empty() && active_ == 0; });
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lk(m_);
            if (stop_) return;
            stop_ = true;
        }
        cv_jobs_.notify_all();
        for (auto& t : threads_) if (t.joinable()) t.join();
        threads_.clear();
    }

private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> jobs_;
    std::mutex m_;
    std::condition_variable cv_jobs_;
    std::condition_variable cv_idle_;
    bool stop_;
    std::size_t active_;
};