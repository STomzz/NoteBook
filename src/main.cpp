#include <thread>
#include <iostream>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <functional>

class ThreadPools
{
private:
    int _threadsNum;
    bool _Stop;
    std::mutex _mtx;
    std::condition_variable _condition;
    std::vector<std::thread> _threadpools;
    std::queue<std::function<void()>> _tasksQue;

public:
    ThreadPools(int num);
    ~ThreadPools();

    template <typename F, typename... Args>
    void enqueue(F &&f, Args &&...args);
};

ThreadPools::ThreadPools(int num) : _threadsNum(num), _Stop(false)
{
    for (int i = 0; i < _threadsNum; i++)
    {
        _threadpools.emplace_back([this]()
                                  {
            while(true){
                std::unique_lock<std::mutex> lock(_mtx);
                _condition.wait(lock, [this]()
                               { return !_tasksQue.empty() || _Stop; });
                if(_Stop && _tasksQue.empty()){
                    return;
                }
                //取任务  重点：std::move将左值转化为右值进行构造（禁用了拷贝构造时候需要使用右值进行构造）
                std::function<void()> task (std::move(_tasksQue.front()));
                _tasksQue.pop();
                lock.unlock();
                _condition.notify_one();
                task();
            } });
    }
}

ThreadPools::~ThreadPools()
{
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _Stop = true;
    }
    _condition.notify_all();
    for (auto &t : _threadpools)
    {
        t.join();
    }
}
template <typename F, typename... Args>
void ThreadPools::enqueue(F &&f, Args &&...args)
{
    std::function<void()> task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    {
        std::unique_lock<std::mutex> lock(_mtx);
        _tasksQue.emplace(std::move(task));
    }
    _condition.notify_one();
}
main()
{
    ThreadPools pool(3);

    for (int i = 0; i < 5; i++)
    {
        pool.enqueue([i]()
                     { 
            std::cout << " task " << i << " is running " <<std::endl; 
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << " task " << i << " is done " <<std::endl; });
    }

    return 0;
}