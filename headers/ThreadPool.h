#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <future>

class ThreadPool{
public:
    static void Init(){
        int NUM_THREADS = std::max(1, static_cast<int>(std::thread::hardware_concurrency() - 1));

        instance().num_threads = NUM_THREADS;
        instance().running = true;

        for(int i = 0; i < NUM_THREADS; i++){
            instance().workers.emplace_back([]{
                while(true){
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(instance().mutex);
                        instance().cv.wait(lock, [] {
                            return !instance().running || !instance().tasks.empty();
                        });
                        if(!instance().running && instance().tasks.empty()) return;
                        task = std::move(instance().tasks.front());
                        instance().tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    static void Shutdown(){
        {
            std::lock_guard<std::mutex> lock(instance().mutex);
            instance().running = false;
        }
        instance().cv.notify_all();
        for (auto& w : instance().workers)
            w.join();
        instance().workers.clear();
    }

    static std::future<void> Submit(std::function<void()> f){
        auto task = std::make_shared<std::packaged_task<void()>>(std::move(f));
        auto fut = task->get_future();
        {
            std::lock_guard<std::mutex> lock(instance().mutex);
            instance().tasks.push([task] { (*task)(); });
        }
        instance().cv.notify_one();
        return fut;
    }
    
    static void ParallelFor(int count, const std::function<void(int start, int end)>& work){
        if (count <= 0) return;

        int n_threads = instance().num_threads;
        if (n_threads <= 0 || count < PARALLEL_THRESHOLD) {
            work(0, count);
            return;
        }
        
        int chunk = (count + n_threads - 1) / n_threads;
        std::vector<std::future<void>> futures;
        futures.reserve(n_threads);
 
        for (int t = 0; t < n_threads; t++) {
            int start = t * chunk;
            if (start >= count) break;
            int end = std::min(start + chunk, count);
 
            futures.push_back(Submit([&work, start, end] {
                work(start, end);
            }));
        }
 
        for (auto& f : futures) f.get();
    }

    static constexpr int PARALLEL_THRESHOLD = 2048;
private:
    // Meyer's singleton
    static ThreadPool& instance() {
        static ThreadPool instance;
        return instance;
    }
                            
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mutex;
    std::condition_variable cv;
    int num_threads;
    bool running;
};

#endif